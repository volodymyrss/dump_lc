try:
    import spiacs_config
    have_spiacs_config=True
except ImportError:
    print("no spiacs-config: standalone")
    have_spiacs_config=False

import copy
import tempfile
import os
import subprocess
import re
import resource
import fcntl

import shutil
import shutilwhich

import pandas as pd

class DumpLCException(Exception):
    pass

class BadTarget(DumpLCException):
    def __init__(self,s=""):
        DumpLCException.__init__(self,"Bad Target"+s)

class NoAccess(DumpLCException):
    def __init__(self,s=""):
        DumpLCException.__init__(self,"No access to the data"+s)

class OverRevolution(DumpLCException):
    def __init__(self):
        DumpLCException.__init__(self,"Over revolution")

#class DumpLCInternalException(DumpLCException):
class DumpLCUnhandledException(Exception):
    def __init__(self):
        DumpLCException.__init__(self,"dump_ihklc returned non-zero")

class NoData(DumpLCException):
    def __init__(self,s=""):
        DumpLCException.__init__(self,"No Data (at this time?) "+s)

class ZeroData(DumpLCException):
    def __init__(self,s=""):
        DumpLCException.__init__(self,"Zero Data (at this time?)"+s)

class NoAuxData(DumpLCException):
    def __init__(self):
        DumpLCException.__init__(self,"No Auxiliaty Data (for this time?)")

def get_open_fds():
    fds = []
    for fd in range(3,resource.RLIMIT_NOFILE):
        try:
            flags = fcntl.fcntl(fd, fcntl.F_GETFD)
        except IOError:
            continue

        fds.append(fd)
    return fds

def close_all(f):
    
    def nf(*a,**aa):
        before=get_open_fds()
        r=f(*a,**aa)
        after=get_open_fds()
        diff=[a for a in after if a not in before]
        print "open before and after",before,after,diff
        return r

    return nf

@close_all
def dump_ihklc(utc1,utc2,mode=0,target="ACS",rbp=None,dump_ihklc_path=None):
    if dump_ihklc_path is None:
        if have_spiacs_config:
            dump_ihklc_binary=spiacs_config.dump_ihklc_binary
            dump_ihklc_path=spiacs_config.dump_ihklc_path
        else:
            dump_ihklc_binary=shutil.which("dump_ihklc")
            dump_ihklc_path=os.path.dirname(dump_ihklc_binary)
    else:
        dump_ihklc_binary=dump_ihklc_path+"/dump_ihklc"

    if rbp is None:
        try:
            return dump_ihklc(utc1,utc2,mode=mode,target=target,rbp=os.environ.get('REP_BASE_PROD_CONS'),
                        dump_ihklc_path=dump_ihklc_path)
        except Exception as e:
            print("unable to extract from CONS",e)
            return dump_ihklc(utc1,utc2,mode=mode,target=target,rbp=os.environ.get('REP_BASE_PROD_NRT'),
                        dump_ihklc_path=dump_ihklc_path)

    temp_dir=tempfile.mkdtemp(suffix="ihklc")
    output_file=os.path.join(temp_dir,"output_data.txt")

    command=[dump_ihklc_binary,
            "start_time_utc="+utc1,
            "stop_time_utc="+utc2,
            "target="+target,
            "output="+output_file,
            "orbit_accy=30",
            "mode=%i"%mode]

    env=copy.deepcopy(os.environ)

    print "env PFILES:",env['PFILES']
    env['PFILES']=temp_dir+";"+env['PFILES'].split(";")[-1]
    env['REP_BASE_PROD']=rbp

    print "command:"," ".join(command)
    print "using PFILES:",env['PFILES']


    exception=None
    try:
        output=subprocess.check_output(command,env=env)
        os.remove(os.path.join(temp_dir,"dump_ihklc.par"))
    except subprocess.CalledProcessError as e:
        exception=e
        print "dump_ihklc returns",repr(e)
        output=exception.output

    try:
        result_raw=open(output_file).read()
        result=pd.read_csv( output_file,
                            delim_whitespace=True,
                            names=["ijd","t_rel_s","counts","t_sod_s"],
                            usecols=[0,1,2,3],
                            skiprows=5)
        os.remove(output_file)
    except Exception as e:
        print "could not read temp file?.."
        raise

    os.rmdir(temp_dir)

    print "output:",output

    if re.search("Error_0: unknown target",output):
        raise BadTarget(" "+target)
    
    s=re.search("unable to read.*? skipping",output)
    if s:
        print "found problems: probably no permission to access"
        #raise NoAccess(" probably no permission to access")
    
    if re.search("No ScW files in here",output):
        raise NoData("requested: "+utc1+" ... "+utc2)
    
    s=re.search("Read (.*?) ScW files with 0 bins",output)
    if s:
        raise NoData(" in %s ScW"%s.group(1))

    if re.search("Fatal! Execution failed with exit code -25501",output):
        raise NoAuxData()
    
    if re.search("Fatal! Execution failed with exit code -25502",output):
        raise NoAuxData()
    
    if re.search("Fatal! Execution failed with exit code -2550",output):
        raise NoAuxData()
    
    if re.search("overrevolution!",output):
        raise OverRevolution()

    errors=re.findall("^Error_. *(.*?)\n",output,re.S and re.M)
    if errors!=[]:
        print "noticed unhandled errors in the output:",errors
        raise Exception("\n".join(errors))

    if exception is not None:
        raise DumpLCUnhandledException(repr(exception))
    
    errors=re.findall("^Warn_.*?\n",output,re.S)
    if errors!=[]:
        print "noticed warnings in the output:",errors

    def sfloat(x):
        try:
            return float(x)
        except:
            return 0

 #   print result
    if mode==0 and all([sfloat(a.split()[2])==0 for a in result_raw.split("\n") if len(a.split())>=4]):
        raise ZeroData()

    print "leaving dump_ihklc"

    return result_raw,result,output
