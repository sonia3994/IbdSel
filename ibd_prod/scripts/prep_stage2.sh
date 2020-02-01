#!/bin/bash

source bash/stage2_vars.inc.sh

while getopts "f:" opt; do
    case $opt in
        f)
            filter_cmd=$OPTARG
            ;;
        *)
            exit 1
            ;;
    esac
done
shift $((OPTIND-1))

tag=$1; shift
if [ -z $tag ]; then
    echo "Specify a tag!"
    exit 1
fi

configname=$1; shift
if [ -z $configname ]; then
    echo "Specify a config!"
    exit 1
fi

stage2_vars $tag $configname

mkdir -p $indir $logdir $trueOutdir
mkdir -p $(dirname $outdir)
ln -s $trueOutdir $outdir

cp $IBDSEL_CONFIGDIR/config.$configname.txt $indir

filter_cmd=${filter_cmd:-cat}

python/dump_days.py | $filter_cmd > $infile