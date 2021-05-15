for file in ../testcases/mix/*.tig
do
    echo "$file: "
    ./parsetest $file 2>&1
done