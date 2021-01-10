# Create a venv and run a ven
pipenv --python 3.7


#Create python wheels
python -m build --sdist --wheel

# Create the wheel
python -m auditwheel repair dist/fastspellchecker-0.0.1-cp38-cp38-linux_x86_64.whl --plat manylinux2014_x86_64
