CPP_BUILD_DIR = build/cpp-service

.PHONY: build proto cpp rust python test test-cpp test-rust test-python
.RECIPEPREFIX = >

build:
> ./scripts/gen-protos.sh
> cmake -S cpp-service -B $(CPP_BUILD_DIR)
> cmake --build $(CPP_BUILD_DIR)
> python -m py_compile python-worker/*.py

test:
> ./scripts/gen-protos.sh
> cmake -S cpp-service -B $(CPP_BUILD_DIR)
> cmake --build $(CPP_BUILD_DIR)
> pip install .
> ctest --test-dir $(CPP_BUILD_DIR)
> pytest || [ $$? -eq 5 ]
> python -m py_compile python-worker/*.py
