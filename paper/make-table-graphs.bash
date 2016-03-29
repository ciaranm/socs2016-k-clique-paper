#!/bin/bash
# vim: set sw=4 sts=4 et :

LIST=$1
DIR=$2
FORMAT=$3

RESULTS_ccon=~/phd/results/2014-03-26-savage/
HOST_ccon=savage

RESULTS_cconlgd=~/phd/results/2014-03-26-savage/
HOST_cconlgd=savage

BIN=~/parasols/build/`hostname`/

bestmethod=cconlgd

cat $1 | while read filename name ; do

    for power in 2 3 4 ; do
        if [[ $power == 2 ]] ; then
            echo $name
        elif [[ $power == 3 ]] ; then
            # |V|
            echo '\hspace*{0.5em}\color{gray}$|V|{=}'$($BIN/about_graph --format $FORMAT $DIR/$filename | cut -d' ' -f1)'$'
        elif [[ $power == 4 ]] ; then
            # |E|
            echo '\hspace*{0.5em}\color{gray}$|E|{=}'$($BIN/about_graph --format $FORMAT $DIR/$filename | cut -d' ' -f2)'$'
        fi

        echo "& $power"

        # density
        echo "&"
        read v e <<<$(<${RESULTS_ccon}/${HOST_ccon}-kclique-$filename-$power.about)
        ruby -e "printf('%.2f', (2.0 * $e) / ($v * ($v - 1.0)))"

        # omega
        echo "&"
        resultsdir=RESULTS_$bestmethod
        resultshost=HOST_$bestmethod
        FILE=${!resultsdir}/${!resultshost}-kclique-$filename-$power-deg-$bestmethod.out
        if grep -q aborted $FILE ; then
            echo '${\ge}'$(head -n1 $FILE | cut -d' ' -f1)'$'
        else
            head -n1 $FILE | cut -d' ' -f1
        fi

        for method in "ccon" "cconlgd" ; do
            resultsdir=RESULTS_$method
            resultshost=HOST_$method
            FILE=${!resultsdir}/${!resultshost}-kclique-$filename-$power-deg-$method.out

            if [[ -f $FILE ]] ; then
                if grep -q aborted $FILE ; then
                    rc='\color{gray}'
                else
                    rc=
                fi
            else
                resultsdir=RESULTS_${method}ALT
                resultshost=HOST_${method}ALT
                FILE=${!resultsdir}/${!resultshost}-kclique-$filename-$power-deg-$method.out
                if [[ -f $FILE ]] ; then
                    if grep -q aborted $FILE ; then
                        rc='\itshape\color{gray}'
                    else
                        rc='\itshape'
                    fi
                else
                    rc=
                fi
            fi

            # nodes
            echo "&"
            echo $rc
            n=$(head -n1 $FILE | cut -d' ' -f2)
            if [[ $n -ge 1000000 ]] ; then
                ruby -e "printf(\"\\\\num{%.1e}\", $n)"
            else
                echo $n
            fi

            # time
            echo "&"
            echo $rc
            t=$(tail -n1 $FILE | cut -d' ' -f1 )
            if [[ $t -ge 86400000 ]] ; then
                echo '$>$ 1 day'
            else
                [[ -s $FILE ]] && ruby -e "printf(\"%.1f \", $t / 1000.0)"
            fi
        done

        echo
        if [[ $power == 4 ]] ; then
            echo "\\\\[0.2cm]"
        else
            echo "\\\\*"
        fi
    done
done | sed -e '$d'

echo "\\\\"

