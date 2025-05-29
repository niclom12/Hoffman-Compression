if [ $# -eq 0 ]; then
  echo "No arguments provided."
else
    make clean
    make 
    touch $3
    ./bin/compression  $1 $2 $3
  
fi