for file in set-1/*.tig
do
    ./../src/tc -p $file
done

for file in set-2/*.tig
do
    ./../src/tc -p $file
done

for file in set-3/*.tig
do
    ./../src/tc -p $file
done