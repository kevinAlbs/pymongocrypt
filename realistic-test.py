from bson import json_util
import ssl
import socket
import pymongocrypt
import pymongo
import bson
from bson import SON
import os
from bson.codec_options import CodecOptions
client = pymongo.MongoClient()


def satisfy_kms_request(kms):
    hostname = 'kms.us-east-1.amazonaws.com'
    context = ssl.create_default_context()
    bin = pymongocrypt.Binary()
    kms.message(bin)
    with socket.create_connection((hostname, 443)) as sock:
        with context.wrap_socket(sock, server_hostname=hostname) as ssock:
            ssock.sendall(bin.data())
            data = ssock.recv()
            while data:
                bin = pymongocrypt.Binary(data)
                kms.feed(bin)
                data = ssock.recv()


codec_options = CodecOptions(uuid_representation=bson.binary.STANDARD)

opt_type = {
    "MONGOCRYPT_AWS_REGION": 0,
    "MONGOCRYPT_AWS_SECRET_ACCESS_KEY": 1,
    "MONGOCRYPT_AWS_ACCESS_KEY_ID": 2,
    "MONGOCRYPT_LOG_FN": 3,
    "MONGOCRYPT_LOG_CTX": 4
}

states = [
    "MONGOCRYPT_CTX_ERROR",
    "MONGOCRYPT_CTX_NOTHING_TO_DO",
    "MONGOCRYPT_CTX_NEED_MONGO_COLLINFO",
    "MONGOCRYPT_CTX_NEED_MONGO_MARKINGS",
    "MONGOCRYPT_CTX_NEED_MONGO_KEYS",
    "MONGOCRYPT_CTX_NEED_KMS",
    "MONGOCRYPT_CTX_READY",
    "MONGOCRYPT_CTX_DONE"
]

opts = pymongocrypt.Opts()
opts.set(opt_type["MONGOCRYPT_AWS_REGION"], os.environ["AWS_REGION"])
opts.set(opt_type["MONGOCRYPT_AWS_ACCESS_KEY_ID"],
         os.environ["AWS_ACCESS_KEY_ID"])
opts.set(opt_type["MONGOCRYPT_AWS_SECRET_ACCESS_KEY"],
         os.environ["AWS_SECRET_ACCESS_KEY"])
client.__crypt = pymongocrypt.Mongocrypt(opts)
client.__mongocryptd_client = pymongo.MongoClient("mongodb://localhost:27020")


def run_machine(ctx, cmd, db, client):
    bin = pymongocrypt.Binary()
    while (True):
        if (states[ctx.state()] == "MONGOCRYPT_CTX_ERROR"):
            raise Exception("libmongocrypt returned an error")
        elif (states[ctx.state()] == "MONGOCRYPT_CTX_NOTHING_TO_DO"):
            return cmd
        elif (states[ctx.state()] == "MONGOCRYPT_CTX_NEED_MONGO_COLLINFO"):
            # get listCollections filter
            ctx.mongo_op(bin)
            filter = bson.BSON.decode(bin.data())
            results = list(client[db].list_collections(filter=filter))
            if len(results) == 0:
                print("pymongo - collection does not have a schema")
                return cmd
            bin = pymongocrypt.Binary(bson.BSON.encode(
                results[0], codec_options=codec_options))
            ctx.mongo_feed(bin)
            ctx.mongo_done()
        elif (states[ctx.state()] == "MONGOCRYPT_CTX_NEED_MONGO_MARKINGS"):
            # get the json schema to append
            ctx.mongo_op(bin)
            schema = bson.BSON.decode(bin.data())
            cmd_name = next(iter(cmd))
            cmd_val = next(iter(cmd.values()))
            # TODO: can I avoid this shallow copy?
            tmp = cmd.copy()
            tmp["jsonSchema"] = schema
            del(tmp[cmd_name])
            response = client.__mongocryptd_client["admin"].command(
                cmd_name, cmd_val, **tmp, codec_options=codec_options)
            bin = pymongocrypt.Binary(bson.BSON.encode(
                response, codec_options=codec_options))
            ctx.mongo_feed(bin)
            ctx.mongo_done()
        elif states[ctx.state()] == "MONGOCRYPT_CTX_NEED_MONGO_KEYS":
            # get the key filter
            ctx.mongo_op(bin)
            filter = bson.BSON.decode(bin.data())
            for doc in client["admin"]["datakeys"].find(filter):
                bin = pymongocrypt.Binary(bson.BSON.encode(
                    doc, codec_options=codec_options))
                ctx.mongo_feed(bin)
            ctx.mongo_done()
        elif states[ctx.state()] == "MONGOCRYPT_CTX_NEED_KMS":
            kms = ctx.next_kms_ctx()
            while kms is not None:
                satisfy_kms_request(kms)
                kms = ctx.next_kms_ctx()
            ctx.kms_done()
        elif states[ctx.state()] == "MONGOCRYPT_CTX_READY":
            ctx.finalize(bin)
            final = bson.BSON.decode(bin.data())
            return final
        else:
            print("terminal state hit: {}".format(states[ctx.state()]))
            break


def auto_encrypt(cmd, db, client):
    coll = next(iter(cmd.values()))
    ns = db + "." + coll
    ctx = client.__crypt.encrypt(ns)
    return run_machine(ctx, cmd, db, client)


def auto_decrypt(doc, db, client):
    bin = pymongocrypt.Binary(bson.BSON.encode(doc))
    ctx = client.__crypt.decrypt(bin)
    return run_machine(ctx, cmd, db, client)


cmd = SON([("find", "crypt"), ("filter", SON([("ssn", "457-55-5462")]))])
encrypted = auto_encrypt(cmd, "test", client)
print ("encrypted to: {}".format(encrypted))
decrypted = auto_decrypt(encrypted, "test", client)
print ("decrypted to: {}".format(decrypted))
