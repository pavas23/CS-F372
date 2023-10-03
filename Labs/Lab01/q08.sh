# bash program to execute a c binary file by passing command line arguments
# $1 is the first argument
out=`./a.out $1`
echo $?
echo "$out"

