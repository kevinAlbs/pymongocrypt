from pymongo import MongoClient
from bson import json_util

json_util.loads()
cmd = {
    "create": "test",
    "validator": {
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
                        "type": "string",
                        "algorithm": "Deterministic",
                        "iv": {
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
    }
}
opts = {
    
};
db.createCollection("test", opts)