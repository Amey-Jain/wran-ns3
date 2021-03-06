#! /usr/bin/env python
from __future__ import unicode_literals, print_function
import sys
import warnings

import pybindgen
import pybindgen.settings
from pybindgen import (FileCodeSink)

import foomodulegen_generated
import foomodulegen_common



class ErrorHandler(pybindgen.settings.ErrorHandler):
    def handle_error(self, wrapper, exception, dummy_traceback_):
        warnings.warn("exception >>> %r in wrapper %s" % (exception, wrapper))
        return True
pybindgen.settings.error_handler = ErrorHandler()



def my_module_gen():
    out = FileCodeSink(sys.stdout)
    root_module = foomodulegen_generated.module_init()
    root_module.add_exception('exception', foreign_cpp_namespace='std', message_rvalue='%(EXC)s.what()')

    ## this is a very special case when we want to change the name of
    ## the python module to allow parallel testing of the same basic
    ## module 'foo' generated by 3 different code paths; normally
    ## users don't need this.
    root_module.name = 'foo3'

    foomodulegen_generated.register_types(root_module)
    foomodulegen_generated.register_methods(root_module)
    foomodulegen_generated.register_functions(root_module)
    foomodulegen_common.customize_module(root_module)

    root_module.generate(out)


if __name__ == '__main__':
    try:
        import cProfile as profile
    except ImportError:
        my_module_gen()
    else:
        print("** running under profiler", file=sys.stderr)
        profile.run('my_module_gen()', 'foomodulegen3.pstat')

