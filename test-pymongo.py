from pymongo import MongoClient

client = MongoClient()

client.db.coll.insert({"x": 1})