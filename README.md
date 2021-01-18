# FastSpellChecker
A dependancy-free c++ Spell Checker with Python bindings 



# Getting started in Python


1. Load a list of words
2. Use ``best_match(word, distance)`` to get a *match* object with the closest word


```
from FastSpellChecker import Dictionary

d = Dictionary()
d.load(["prout", "pret", "part", "tourte"])
m = d.best_match("pro", 2)
assert m["count"] == 3
assert m["distance"] == 2
assert m["word"] in ["prout", "pret", "part"]
```

A **Match** contains 3 things:


* distance: The distance to the best match
* word: The best matching word (or one of them, if multiple matches)
* count: The number of matches with this distance


# Install

```
pip install -i https://test.pypi.org/simple/ fastspellchecker
```


# Limitations

* Only ASCII (or any 8-bits encoding like Latin-1) handled

