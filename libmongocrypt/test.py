import bson
c = bson.BSON.encode({"x":1})

import pymongocrypt
pymongocrypt.setup("us-east-1", "access key", "secret key")