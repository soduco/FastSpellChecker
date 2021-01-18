from FastSpellChecker import Dictionary

def test_0():
    d = Dictionary()
    d.load(["prout", "pret", "part", "tourte"])
    m = d.best_match("tour", 2)
    assert m["word"] == "tourte"
    assert m["distance"] == 2

def test_insertion():
    d = Dictionary()
    d.load(["prout", "pret", "part", "tourte"])
    m = d.best_match("pet", 2)
    assert m["word"] == "pret"
    assert m["distance"] == 1

def test_deletion():
    d = Dictionary()
    d.load(["prout", "pret", "part", "tourte"])
    m = d.best_match("parti", 2)
    assert m["word"] == "part"
    assert m["distance"] == 1

def test_substitution():
    d = Dictionary()
    d.load(["prout", "pret", "part", "tourte"])
    m = d.best_match("port", 2)
    assert m["word"] == "part"
    assert m["distance"] == 1

def test_edit_2():
    d = Dictionary()
    d.load(["prout", "pret", "part", "tourte"])
    m = d.best_match("pro", 2)
    assert m["count"] == 3
    assert m["distance"] == 2
    assert m["word"] in ["prout", "pret", "part"]

def test_no_result():
    d = Dictionary()
    d.load(["prout", "pret", "part", "tourte"])
    #m = d.best_match("pro", 1)
    #assert m is None

