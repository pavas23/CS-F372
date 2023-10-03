# $# gives number of command line args
# $@ is the array of all command line args

echo "number of args passed: $#"
for i in $@
do
    echo $i
done

