#!/bin/bash

# Make sure we exit if there is a failure
set -e

usage() {
    echo "Usage: $0 FILE [-inline]"
}

if [[ $# -lt 1 ]]; then
    usage
    exit 1
fi

INLINE=false 
POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"
case $key in
    -inline|--inline)
	shift # past argument
	INLINE=true
	;;
    -help|--help)
	usage
	exit 0
	;;
    *)  # unknown option
	POSITIONAL+=("$1") # save it in an array for later
	shift # past argument
	;;
esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters
FILE=$1
shift

INSTALL_DIR=$(pwd)/../install/bin

CLAMPP=${INSTALL_DIR}/clam-pp
if [ "${CLAMPP}" == "" ]; then
    echo "Cannot find clam-pp"
    exit 1
fi    

SEAOPT=${INSTALL_DIR}/seaopt
if [ "${SEAOPT}" == "" ]; then
    echo "Cannot find seaopt"
    exit 1
fi    

NTTVERIFIER=${INSTALL_DIR}/nttverifier
if [ "${NTTVERIFIER}" == "" ]; then
    echo "Cannot find nttverifier"
    exit 1
fi    

### Clam preprocessor
CLAMPP_OPTS="--simplifycfg-sink-common=false --clam-devirt --devirt-resolver=sea-dsa --sea-dsa-type-aware=true"
if [ ${INLINE} == true ] ; then
CLAMPP_OPTS="${CLAMPP_OPTS} --clam-inline-all" 
fi    
${CLAMPP} ${FILE} ${CLAMPP_OPTS} -o ${FILE}.pp.bc
### Static loop unrolling
${SEAOPT} -O1  ${FILE}.pp.bc \
     -loop-simplify -fake-latch-exit -loop-unroll -unroll-threshold=99999999 \
     -o ${FILE}.unrolled.pp.bc	       
### NTT Verifier
${NTTVERIFIER} ${FILE}.unrolled.pp.bc

exit 0
