from pymongo import MongoClient
import bson
from bson import json_util

json_options = json_util.JSONOptions(
    uuid_representation=bson.binary.STANDARD)

codec_options = json_util.CodecOptions(
    uuid_representation=bson.binary.STANDARD)

validator = """{
    "$jsonSchema": {
        "properties": {
            "ssn": {
                "encrypt": {
                    "keyId": [{
                        "$binary": {
                            "base64": "AAAAAAAAAAAAAAAAAAAAAA==",
                            "subType": "04"
                        }
                    }],
                    "bsonType": "string",
                    "algorithm": "AEAD_AES_256_CBC_HMAC_SHA_512-Deterministic",
                    "initializationVector": {
                            "$binary": {
                                "base64": "aWlpaWlpaWlpaWlpaWlpaQ==",
                                "subType": "00"
                            }
                    }
                }
            }
        },
        "bsonType": "object"
    }
}"""

key_doc = """{
    "status": {
        "$numberInt": "1"
    }, 
    "_id": {
        "$binary": {
            "base64": "AAAAAAAAAAAAAAAAAAAAAA==", 
            "subType": "04"
        }
    }, 
    "masterKey": {
        "awsKey": "arn:aws:kms:us-east-1:579766882180:key/89fcc2c4-08b0-4bd9-9f25-e30687b580d0", 
        "awsRegion": "us-east-1", 
        "provider": "aws"
    }, 
    "updatedDate": {
        "$date": {
            "$numberLong": "1553026537755"
        }
    }, 
    "keyMaterial": {
        "$binary": {
            "base64": "AQICAHhQNmWG2CzOm1dq3kWLM+iDUZhEqnhJwH9wZVpuZ94A8gEdnNDpvEH3aukK7+DVjuYXAAAAojCBnwYJKoZIhvcNAQcGoIGRMIGOAgEAMIGIBgkqhkiG9w0BBwEwHgYJYIZIAWUDBAEuMBEEDIkm/uaBLakm8bozbQIBEIBbvPB7EluCCXGmVRrV7r/wK3lqgb1do4fwRL0Yw2Mbdb+ignWGtWYNssDLT8N+zNm8gUYdoazLu8X5zsUJJdyPhRru85SR0Iw2lt9WdXrH3E0KAkpUeRt4oaQX2w==", 
            "subType": "00"
        }
    }, 
    "creationDate": {
        "$date": {
            "$numberLong": "1553026537755"
        }
    }
}"""

validator = json_util.loads(validator, json_options=json_options)
key_doc = json_util.loads(key_doc, json_options=json_options)

client = MongoClient()
client["test"]["crypt"].drop()
client["admin"]["datakeys"].drop()
client["test"].command("create", "crypt", validator=validator, codec_options=codec_options)
client["admin"]["datakeys"].insert_one(key_doc)