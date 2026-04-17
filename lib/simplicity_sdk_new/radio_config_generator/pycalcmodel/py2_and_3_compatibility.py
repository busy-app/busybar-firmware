import math
import sys
if sys.version_info[0] < 3:
    pass
else:
    # hack for backward compatibility
    long = int
    xrange = range
    basestring = str
    sys.maxint = 9223372036854775807
    import builtins as __builtin__
    try:
        # Python 3.4+ has importlib.reload
        from importlib import reload
    except ImportError:
        # Python 3.0-3.3 fallback to imp.reload
        try:
            import imp
            reload = imp.reload
        except ImportError:
            # If imp is not available (Python 3.12+), define a no-op reload
            def reload(module):
                pass

# try:
#     import __builtin__
# except ModuleNotFoundError:
#     import builtins as __builtin__

def py2round(x):
    if x >= 0.0:
        return math.floor(x + 0.5)
    else:
        return math.ceil(x - 0.5)