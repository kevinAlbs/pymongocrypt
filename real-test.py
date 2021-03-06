from pymongo import MongoClient
import os
import bson

cfg = {
    "kmsProviders": {
        "aws": {
           "region": os.environ["AWS_REGION"],
           "accessKeyId": os.environ["AWS_ACCESS_KEY_ID"],
           "secretAccessKey": os.environ["AWS_SECRET_ACCESS_KEY"]
        }
    }
}

client = MongoClient(client_side_encryption=cfg)
coll = client.test.crypt
coll.insert_one({"name": "Todd Davis", "ssn": "457-55-5462"})
print(coll.find_one())