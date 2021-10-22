# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

from pyis.python import ops

class Model:
    def __init__(self) -> None:
        self.trie1 = ops.CedarTrie()
        self.trie1.insert('what is the weather')
        self.trie1.insert('will it rain')

        self.trie2 = ops.CedarTrie()
        self.trie2.insert('今天天气怎么样')
        self.trie2.insert('明天会下雨吗')

    def forward(self, query: str, locale: str) -> bool:
        if locale == 'en-us':
            return self.trie1.contains(query)
        elif locale == 'zh-cn':
            return self.trie2.contains(query)
        return False

model = Model()
is_weather = model.forward('what is the weather', 'en-us') 
print(is_weather) # True
is_weather = model.forward('what is the answer', 'en-us') 
print(is_weather) # False
is_weather = model.forward('明天会下雨吗', 'zh-cn') 
print(is_weather) # True


import os
from pyis.python import save

# serialize the model
os.makedirs('tmp', exist_ok=True)
model_file = 'tmp/model.pkl'
save(model, model_file)


from pyis.python import load

# de-serialize the model
loaded_model = load(model_file)
is_weather = loaded_model.forward('what is the weather', 'en-us') 
print(is_weather) # True
is_weather = loaded_model.forward('what is the answer', 'en-us') 
print(is_weather) # False
is_weather = loaded_model.forward('明天会下雨吗', 'zh-cn') 
print(is_weather) # True