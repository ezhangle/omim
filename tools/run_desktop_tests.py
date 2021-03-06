#!/usr/bin/env python

"""
This script is mainly for running autotests on the build server, however, it 
can also be used by engineers to run the tests locally on their machines.

It takes as optional parameters the path to the folder containing the test 
executables (which must have names ending in _tests), and a list of tests that 
need to be skipped, this list must be comma separated and contain no spaces. E.g.:

./run_desktop_tests.py -f ./omim-build-release -e drape_tests,some_other_tests

The script outputs the console output of the tests. It also checks the error 
code of each test suite, and after all the tests are executed, it prints the 
list of the failed tests, passed tests, skipped tests and tests that could not 
be found, i.e. the tests that were specified in the skip list, but do not exist.
"""

from __future__ import print_function

from optparse import OptionParser
from os import listdir, remove
from os.path import isfile, join
from random import shuffle
import socket
import subprocess
import sys
import testserver
import urllib2

import logging

TO_RUN = "to_run"
SKIP = "skip"
NOT_FOUND = "not_found"
FAILED = "failed"
PASSED = "passed"

PORT = 34568


class TestRunner:

    def print_pretty(self, result, tests):
        if not tests:
            return
        
        logging.info("\n{result}".format(result=result.upper()))

        for test in tests:
            logging.info("- {test}".format(test=test))


    def set_global_vars(self):

        parser = OptionParser()
        parser.add_option("-o", "--output", dest="output", default="testlog.log", help="resulting log file. Default testlog.log")
        parser.add_option("-f", "--folder", dest="folder", default="omim-build-release/out/release", help="specify the folder where the tests reside (absolute path or relative to the location of this script)")
        parser.add_option("-d", "--data_path", dest="data_path", help="Path to data files (passed to the test executables as --data_path=<value>)")
        parser.add_option("-u", "--user_resource_path", dest="resource_path", help="Path to resources, styles and classificators (passed to the test executables as --user_resource_path=<value>)")
        parser.add_option("-i", "--include", dest="runlist", action="append", default=[], help="Include test into execution, comma separated list with no spaces or individual tests, or both. E.g.: -i one -i two -i three,four,five")
        parser.add_option("-e", "--exclude", dest="skiplist", action="append", default=[], help="Exclude test from execution, comma separated list with no spaces or individual tests, or both. E.g.: -i one -i two -i three,four,five")
        
        (options, args) = parser.parse_args()

        self.skiplist = set()
        self.runlist = list()
        
        for tests in options.skiplist:
            for test in tests.split(","):
                self.skiplist.add(test)
        
        for tests in options.runlist:
            self.runlist.extend(tests.split(","))
            
        if self.runlist:
            logging.warn("-i option found, the -e option will be ignored")
        
        self.workspace_path = options.folder
        self.logfile = options.output
        self.data_path = (" --data_path={0}".format(options.data_path) if options.data_path else "")
        self.user_resource_path = (" --user_resource_path={0}".format(options.resource_path) if options.resource_path else "") 
        

    def start_server(self):
        server = testserver.TestServer()
        server.start_serving()


    def stop_server(self):
        try:
            urllib2.urlopen('http://localhost:{port}/kill'.format(port=PORT), timeout=5)
        except (urllib2.URLError, socket.timeout):
            logging.info("Failed to stop the server...")


    def categorize_tests(self):
            
        tests_to_run = []
        local_skiplist = []
        not_found = []
    
        test_files_in_dir = filter(lambda x: x.endswith("_tests"), listdir(self.workspace_path))

        on_disk = lambda x: x in test_files_in_dir
        not_on_disk = lambda x : not on_disk(x)

        if not self.runlist:
            local_skiplist = filter(on_disk, self.skiplist)
            not_found = filter(not_on_disk, self.skiplist)
            tests_to_run = filter(lambda x: x not in local_skiplist, test_files_in_dir)
        else:
            tests_to_run = filter(on_disk, self.runlist)
            shuffle(tests_to_run)
            
            not_found = filter(not_on_disk, self.runlist)

        return {TO_RUN:tests_to_run, SKIP:local_skiplist, NOT_FOUND:not_found}        
        

    def run_tests(self, tests_to_run):
        failed = []
        passed = []

        for test_file in tests_to_run:
            
            self.log_exec_file(test_file)

            if test_file == "platform_tests":
                self.start_server()
        
            test_file_with_keys = "{test_file}{data}{resources}".format(test_file=test_file, data=self.data_path, resources=self.user_resource_path)
        
            logging.info(test_file_with_keys)
            process = subprocess.Popen("{tests_path}/{test_file} 2>> {logfile}".
                                   format(tests_path=self.workspace_path, test_file=test_file_with_keys, logfile=self.logfile),
                                   shell=True,
                                   stdout=subprocess.PIPE)

            process.wait()

            if test_file == "platform_tests":
                self.stop_server()

            if process.returncode > 0:
                failed.append(test_file)
            else:
                passed.append(test_file)
            
            self.log_exec_file(test_file, result=process.returncode)

        return {FAILED: failed, PASSED: passed}


    def log_exec_file(self, filename, result=None):
        logstring = "BEGIN" if result is None else "END"  #can be 0 or None. If we omit the explicit check for None, we get wrong result
        resstring = (" | result: {returncode}".format(returncode=result) if result is not None else "")
        with open(self.logfile, "a") as logf: 
            logf.write("\n{logstring}: {filename}{resstring}\n".format(logstring=logstring, filename=filename, resstring=resstring))


    def rm_log_file(self):
        try:
            remove(self.logfile)
        except OSError:
            pass


    def __init__(self):
        self.set_global_vars()
        self.rm_log_file()


    def execute(self):
        categorized_tests = self.categorize_tests()

        results = self.run_tests(categorized_tests[TO_RUN])

        self.print_pretty("failed", results[FAILED])
        self.print_pretty("skipped", categorized_tests[SKIP])
        self.print_pretty("passed", results[PASSED])
        self.print_pretty("not found", categorized_tests[NOT_FOUND])


runner = TestRunner()
runner.execute()
