module := mizar

submodules := tests
-include $(patsubst %, $(module)/%/module.mk, $(submodules))

GRPC_FLAGS := -I mizar/proto/ --python_out=. --grpc_python_out=.

.PHONY: proto
proto:
	python3 -m grpc_tools.protoc $(GRPC_FLAGS) mizar/proto/mizar/proto/*.proto

clean::
	rm -rf mizar/proto/__pycache__
	find mizar/proto/ -name '*.py' -not -name '__init__.py' -delete