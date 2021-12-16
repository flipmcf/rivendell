#!%PYTHON_BANGPATH%

# lock_log.py
#
# RivWebPyApi test script for Rivendell
#
#  Test the LockLog Web API call
#
#   (C) Copyright 2021 Fred Gleason <fredg@paravelsystems.com>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU Lesser General Public License version 2 as
#   published by the Free Software Foundation.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

import getpass
import sys

from rivwebpyapi import rivwebpyapi

def eprint(*args,**kwargs):
    print(*args,file=sys.stderr,**kwargs)

url='';
username=''
password=''
log_name=''
operation=''
guid=''

#
# Get login parameters
#
usage='lock_log --url=<rd-url> --username=<rd-username> --log-name=<str> --operation=CREATE|UPDATE|CLEAR --guid=<str>'
for arg in sys.argv:
    f0=arg.split('=')
    if(len(f0)==2):
        if(f0[0]=='--url'):
            url=f0[1]
        if(f0[0]=='--username'):
            username=f0[1]
        if(f0[0]=='--password'):
            password=f0[1]
        if(f0[0]=='--log-name'):
            log_name=f0[1]
        if(f0[0]=='--operation'):
            operation=f0[1]
        if(f0[0]=='--guid'):
            guid=f0[1]

if(not password):
    password=getpass.getpass()
if((not url)or(not username)):
    eprint(usage)
    sys.exit(1)
if(not log_name):
    eprint('you must supply "--log-name"')
    sys.exit(1)
if(not operation):
    eprint('you must supply "--operation"')
    sys.exit(1)
if((operation!='CREATE')and(operation!='UPDATE')and(operation!='CLEAR')):
    eprint('invalid "--operation" specified');
    sys.exit(1)
if((operation!='CREATE')and(not guid)):
    eprint('you must supply "--guid"')
    sys.exit(1)

#
# Execute
#
site=rivwebpyapi.Site(url=url,username=username,password=password)
try:
    lock=site.LockLog(log_name=log_name,operation=operation,guid=guid)
except rivwebpyapi.RivWebPyError as err:
    eprint('*** ERROR ***')
    eprint('Response Code: '+str(err.responseCode))
    eprint('ErrorString: '+str(err.errorString))
    eprint('*************')
    eprint('')
    sys.exit(1)

#
# Display the result
#
for key in lock.values():
    print(key+': '+str(lock.values()[key]))
print('')
