import pymongocrypt
import bson
from bson.json_util import loads
from bson.codec_options import CodecOptions
import os

print(pymongocrypt.version())

opt_type = {
   "MONGOCRYPT_AWS_REGION" : 0,
   "MONGOCRYPT_AWS_SECRET_ACCESS_KEY" : 1,
   "MONGOCRYPT_AWS_ACCESS_KEY_ID" : 2,
   "MONGOCRYPT_LOG_FN" : 3,
   "MONGOCRYPT_LOG_CTX" : 4
}

def print_state(ctx):
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
    print("\nlibmongocrypt in state: {}".format(states[ctx.state()]))


code_options = CodecOptions(uuid_representation=bson.binary.STANDARD)

opts = pymongocrypt.Opts()
opts.set(opt_type["MONGOCRYPT_AWS_REGION"], os.environ["AWS_REGION"])
opts.set(opt_type["MONGOCRYPT_AWS_ACCESS_KEY_ID"], os.environ["AWS_ACCESS_KEY_ID"])
opts.set(opt_type["MONGOCRYPT_AWS_SECRET_ACCESS_KEY"], os.environ["AWS_SECRET_ACCESS_KEY"])

mc=pymongocrypt.Mongocrypt(opts)

bin=pymongocrypt.Binary()

e=mc.encrypt("test.test")

print_state(e)

e.mongo_op(bin)

print("\nlibmongocrypt gave filter: {}".format(bson.BSON.decode(bin.data())))

collinfo=loads(open("./data/collection-info.json", "r").read())

bin=pymongocrypt.Binary(bson.BSON.encode(collinfo, codec_options=code_options))

e.mongo_feed(bin)

e.mongo_done()

print_state(e)

e.mongo_op(bin)

print("\nlibmongocrypt gave schema: {}".format(bson.BSON.decode(bin.data())))

marked=loads(open("./data/mongocryptd-reply.json", "r").read())

bin=pymongocrypt.Binary(bson.BSON.encode(marked, codec_options=code_options))

print("\nfeeding: {}".format(bson.BSON.decode(bin.data())))

e.mongo_feed(bin)

e.mongo_done()

print_state(e)

e.mongo_op(bin)

print("\nlibmongocrypt gave filter: {}".format(bson.BSON.decode(bin.data())))

key=loads(open("./data/key-document.json", "r").read())

bin=pymongocrypt.Binary(bson.BSON.encode(key, codec_options=code_options))

print("\nfeeding: {}".format(bson.BSON.decode(bin.data())))

e.mongo_feed(bin)

e.mongo_done()

kms=e.next_kms_ctx()

print(kms.bytes_needed())

kms.message(bin)

print ("\nsending to KMS: {}".format(bin.data()))

import socket
import ssl

hostname = 'kms.us-east-1.amazonaws.com'
context = ssl.create_default_context()

with socket.create_connection((hostname, 443)) as sock:
    with context.wrap_socket(sock, server_hostname=hostname) as ssock:
        print(ssock.version())
        ssock.sendall(bin.data())
        data = ssock.recv()
        while data:
            print(data)
            bin = pymongocrypt.Binary (data)
            kms.feed(bin)
            data = ssock.recv()


print("need: {} bytes".format(kms.bytes_needed()))
e.kms_done()

e.finalize(bin)

print(bson.BSON.decode(bin.data()))
