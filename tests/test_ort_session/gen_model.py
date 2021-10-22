import torch
import onnxruntime as ort
import numpy as np

@torch.jit.script
def get_label(tokens: torch.Tensor):
    if tokens[0] == 49061:
        return torch.ones(1, dtype=torch.bool)
    else:
        return torch.zeros(1, dtype=torch.bool)

class Model(torch.nn.Module):
    def forward(self, tokens: torch.Tensor):
        label = get_label(tokens)
        return label

m = Model()
tokens = torch.randint(0, 100000, (10,))
label = m(tokens)
torch.onnx.export(m, (tokens), './data/enus_emotion.onnx', input_names=['tokens'], output_names=['label'], verbose=True)
torch.onnx.export(m, (tokens), './data/intl_emotion.onnx', input_names=['tokens'], output_names=['label'], verbose=True)

ort_sess = ort.InferenceSession('enus_emotion.onnx')
outputs = ort_sess.run(['label'], {
    'tokens': np.array([1, 1, 1, 1, 1, 1, 1, 1, 1, 1], dtype=np.int64)
})
print(outputs)
