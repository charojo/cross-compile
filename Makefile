CPP_BUILD_DIR = build/cpp-service

.PHONY: setup lint build proto cpp rust python test test-cpp test-rust test-python

setup:
	pre-commit install
	ln -sf ../../scripts/pre-push.sh .git/hooks/pre-push

lint:
	pre-commit run --all-files

build:
	./scripts/gen-protos.sh
	cmake -S cpp-service -B $(CPP_BUILD_DIR)
	cmake --build $(CPP_BUILD_DIR)
	python -m py_compile python-worker/worker.py

test:
	./scripts/gen-protos.sh
	cmake -S cpp-service -B $(CPP_BUILD_DIR)
	cmake --build $(CPP_BUILD_DIR)
	ctest --test-dir $(CPP_BUILD_DIR)
	PYTHONPATH=.:$$PYTHONPATH pytest || [ $$? -eq 5 ]
	python -m py_compile python-worker/worker.py
