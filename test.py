import bson
import base64
from bson.json_util import dumps

schema = bson.BSON.encode({
  "bsonType": "object",
  "properties": {
    "ssn": {
      "encrypt": {
        "type": "string",
        "algorithm": "Deterministic",
        "keyId": bson.binary.Binary(base64.b64decode("1+niXaxyRL6AB6xRzUp/Ew=="), bson.binary.UUID_SUBTYPE),
        "iv": bson.binary.Binary(base64.b64decode("YWFhYWFhYWFhYWFhYWFhYQ=="), bson.binary.BINARY_SUBTYPE),
        "keyVaultURI": "mongodb://localhost:27017/admin"
      }
    }
  }
})

doc = bson.BSON.encode({
    "name": "Todd Davis",
    "ssn": "457-55-5642"
})

import sys
import os

if not all(arg in os.environ for arg in ["AWS_REGION", "AWS_ACCESS_KEY_ID", "AWS_SECRET_ACCESS_KEY"]):
    print "Set AWS_REGION, AWS_ACCESS_KEY_ID, and AWS_SECRET_ACCESS_KEY as environment variables."
    sys.exit(1)

import pymongocrypt
print pymongocrypt.setup(os.environ["AWS_REGION"], os.environ["AWS_ACCESS_KEY_ID"], os.environ["AWS_SECRET_ACCESS_KEY"])
data = pymongocrypt.encrypt(schema, doc)

encrypted = bson.BSON.decode(bson.BSON(data))
print ("is result anything?")
print(dumps(encrypted))

data = pymongocrypt.decrypt(data)
print ("and back again")
decrypted = bson.BSON.decode(bson.BSON(data))
print(dumps(decrypted))


