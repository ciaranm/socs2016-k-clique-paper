#!/bin/bash
# vim: set sw=4 sts=4 et :

LIST=$1

RESULTS_cconlgd=../experiments/results/sequential-lgd
RESULTS_tcconlgd=../experiments/results/parallel-lgd
RESULTS_about=../experiments/results/about

cat $1 | while read filename name format ; do

    for power in 2 3 4 ; do
        if [[ $power == 2 ]] ; then
            echo -n $name " "
        elif [[ $power == 3 ]] ; then
            # |V|
            echo -n '\hspace*{0.2em}\color{gray}$|V|{=}'$(../code/about_graph --format $format ../instances/$filename | cut -d' ' -f1)'$'
        elif [[ $power == 4 ]] ; then
            # |E|
            echo -n '\hspace*{0.2em}\color{gray}$|E|{=}'$(../code/about_graph --format $format ../instances/$filename | cut -d' ' -f2)'$'
        fi

        echo -n "& $power"

        # density
        echo -n "&"
        read v e x < ${RESULTS_about}-${power}/$name.out
        ruby -e "printf('%.2f', (2.0 * $e) / ($v * ($v - 1.0)))"

        # omega
        echo -n "&"
        FILE=${RESULTS_tcconlgd}-${power}/$name.out
        if grep -q aborted $FILE ; then
            echo -n '${\ge}'$(head -n1 $FILE | cut -d' ' -f1)'$'
        else
            echo -n $(head -n1 $FILE | cut -d' ' -f1 )
        fi

        for method in "cconlgd" "tcconlgd" ; do
            resultsdir=RESULTS_$method
            FILE=${!resultsdir}-${power}/$name.out

            if [[ -f $FILE ]] ; then
                if grep -q aborted $FILE ; then
                    rc='\color{gray}'
                else
                    rc=
                fi
            fi

            # nodes
            echo -n "&"
            echo -n $rc
            n=$(head -n1 $FILE | cut -d' ' -f2)
            if [[ $n -ge 1000000 ]] ; then
                ruby -e "printf(\"\\\\num{%.1e}\", $n)"
            else
                echo -n $n
            fi

            # time
            echo -n "&"
            echo -n $rc
            t=$(tail -n1 $FILE | cut -d' ' -f1 )
            if grep -q aborted $FILE && [[ $t -ge 3600000 ]] && [[ $t -le 3610000 ]] ; then
                echo -n '$>$ 1 h'
            elif grep -q aborted $FILE && [[ $t -ge 43200000 ]] && [[ $t -le 43201000 ]] ; then
                echo -n '$>$ 12 h'
            else
                [[ -s $FILE ]] && ruby -e "printf(\"%.1f \", $t / 1000.0)"
            fi
        done

        echo "\\\\"
    done
done

