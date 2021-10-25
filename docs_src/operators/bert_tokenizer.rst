===========================
BertTokenizer
===========================

BertTokenizer operator is an adapted version from official Hugging face BertTokenizerFast implementation.

Summary of the tokenizers (🤗 Huggingface_)
========================================================

.. _Huggingface: https://huggingface.co/transformers/tokenizer_summary.html#summary-of-the-tokenizers

Tokenizing a text is splitting it into words or subwords, which then are converted to ids through a look-up table. Converting words or subwords to ids is straightforward. 

Space and punctuation tokenization and rule-based tokenization are both examples of word tokenization, which is loosely defined as splitting sentences into words. While it's the most intuitive way to split texts into smaller chunks, this tokenization method can lead to problems for massive text corpora. In this case, space and punctuation tokenization usually generates a very big vocabulary (the set of all unique words and tokens used). E.g., Transformer XL uses space and punctuation tokenization, resulting in a vocabulary size of 267,735!

While character tokenization is very simple and would greatly reduce memory and time complexity it makes it much harder for the model to learn meaningful input representations. E.g. learning a meaningful context-independent representation for the letter "t" is much harder than learning a context-independent representation for the word "today". Therefore, character tokenization is often accompanied by a loss of performance. So to get the best of both worlds, transformers models use a hybrid between word-level and character-level tokenization called subword tokenization.

There are three main types of tokenizers used in Transformers: 

- Byte-Pair Encoding (BPE)
- WordPiece, used by BERT, DistilBERT, and Electra
- SentencePiece

APIs
===========================

.. autoclass:: pyis.python.ops.BertTokenizer
    :members:
    :undoc-members:

Example
============================

.. literalinclude:: ../examples/doc_bert_tokenizer.py
    :language: python
