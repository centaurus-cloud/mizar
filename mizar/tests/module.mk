mgmt_tests:: all
	sudo python3 -W ignore -m unittest discover mizar/tests/ -v

.PHONY: run_mgmt_test
run_mgmt_test:
	sudo python3 -W ignore -m unittest mizar.tests.${TEST} -v

clean::
	rm -rf mizar/test/__pycache__
	rm -rf mizar/test/*.pyc
	rm -rf mizar/common/__pycache__
	rm -rf mizar/common/*.pyc
