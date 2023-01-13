#!/bin/bash

# Keep track of all of the test results
IMG_DIR="./student_images/"
TEST_IMGS=("inclass1.img" "inclass2.img" "1.img" "2.img" "3.img" "4.img" "5.img" "6.img" "7.img" "8.img" "9.img" "10.img" "11.img" "12.img" "13.img" "14.img" "15.img" "16.img" "20.img")
make

# Loop through all of the test results
# And run then via ./a.out -f <filename>
for img in ${TEST_IMGS[@]}; do
    FILE=${IMG_DIR}${img}
    echo "============================== ${img} =============================="
    ./a.out -f ${FILE}
done