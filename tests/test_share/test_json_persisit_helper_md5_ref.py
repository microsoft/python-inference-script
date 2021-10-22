import hashlib
import json
from enum import IntEnum
  
class JsonNodeType(IntEnum):
    NONE = 0
    BOOL = 1
    INT = 2
    UINT = 3
    INT64 = 4
    UINT64 = 5
    FLOAT = 6
    DOUBLE = 7
    STR = 8
    ARR = 9
    DICT = 10
    FILE = 11

def hex_digest(obj, md5):
    if isinstance(obj, bool):
        md5.update(int(JsonNodeType.BOOL).to_bytes(4, byteorder='little', signed=True))
        md5.update(int(obj).to_bytes(4, byteorder='little', signed=True))
    elif isinstance(obj, int):
        if obj >= -2147483648 and obj <= 0x7FFFFFFF:
            md5.update(int(JsonNodeType.INT).to_bytes(4, byteorder='little', signed=True))
            md5.update(obj.to_bytes(4, byteorder='little', signed=True))
        elif obj >=0 and obj <= 0xFFFFFFFF:
            md5.update(int(JsonNodeType.UINT).to_bytes(4, byteorder='little', signed=True))
            md5.update(obj.to_bytes(4, byteorder='little', signed=False))          
        elif obj >= -9223372036854775808 and obj <= 0x7FFFFFFFFFFFFFFF:
            md5.update(int(JsonNodeType.INT64).to_bytes(4, byteorder='little', signed=True))
            md5.update(obj.to_bytes(8, byteorder='little', signed=True))
        elif obj >=0 and obj <= 0xFFFFFFFFFFFFFFFF:
            md5.update(int(JsonNodeType.UINT64).to_bytes(4, byteorder='little', signed=True))
            md5.update(obj.to_bytes(8, byteorder='little', signed=False))
        else:
            raise RuntimeError(f"unsupported integer {obj}")       
    elif isinstance(obj, float):
        md5.update(int(JsonNodeType.FLOAT).to_bytes(4, byteorder='little', signed=True))
        md5.update(obj.to_bytes(8, byteorder='little', signed=True))
    elif isinstance(obj, str):
        md5.update(int(JsonNodeType.STR).to_bytes(4, byteorder='little', signed=True))
        md5.update(len(obj).to_bytes(8, byteorder='little', signed=False))
        md5.update(obj.encode('ascii'))
    elif isinstance(obj, list):
        md5.update(int(JsonNodeType.ARR).to_bytes(4, byteorder='little', signed=True))
        md5.update(len(obj).to_bytes(8, byteorder='little', signed=False))
        for i in obj:
            hex_digest(i, md5)
    elif isinstance(obj, dict):
        md5.update(int(JsonNodeType.DICT).to_bytes(4, byteorder='little', signed=True))
        md5.update(len(obj).to_bytes(8, byteorder='little', signed=False))
        for k in sorted (obj.keys()):
            # key
            if k.endswith(':file'):
                md5.update(int(JsonNodeType.FILE).to_bytes(4, byteorder='little', signed=True))
            else:
                md5.update(int(JsonNodeType.STR).to_bytes(4, byteorder='little', signed=True))
            md5.update(len(k).to_bytes(8, byteorder='little', signed=False))
            md5.update(k.encode('ascii'))

            # value
            if k.endswith(':file'):
                with open(obj[k], "rb") as f:
                    bytes_read = f.read()
                    md5.update(bytes_read)
            else:
                hex_digest(obj[k], md5)
    elif obj is None:
        md5.update(int(JsonNodeType.NONE).to_bytes(4, byteorder='little', signed=True))

def sign(obj):
    hash = hashlib.md5()
    hex_digest(obj, hash)
    return hash.hexdigest()

obj = {"version": 1}
print(json.dumps(obj))
print(sign(obj))

obj = {"version": 2, "die": True}
print(json.dumps(obj))
print(sign(obj))

obj = {"version": 1, "answer": 42}
print(json.dumps(obj))
print(sign(obj))

obj = {"version": 1, "max_uint": 0xffffffff}
print(json.dumps(obj))
print(sign(obj))

obj = {"version": 1, "min_int64": -9223372036854775808}
print(json.dumps(obj))
print(sign(obj))

obj = {"version": 1, "max_uint64": 0xFFFFFFFFFFFFFFFF}
print(json.dumps(obj))
print(sign(obj))

obj = {"version": 1, "key": "value"}
print(json.dumps(obj))
print(sign(obj))

obj = {"version": 1, "list": [1,2,3]}
print(json.dumps(obj))
print(sign(obj))

obj = {"version": 1, "config:file": 'tests/test_share/data/file_without_newline.for_cache_test.txt'}
print(json.dumps(obj))
print(sign(obj))