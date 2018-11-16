echo "start"

g++ train.cpp -o train

echo "train with 900 iterations"

./train 900 model_init.txt seq_model_01.txt model_01.txt
./train 900 model_init.txt seq_model_02.txt model_02.txt
./train 900 model_init.txt seq_model_03.txt model_03.txt
./train 900 model_init.txt seq_model_04.txt model_04.txt
./train 900 model_init.txt seq_model_05.txt model_05.txt

echo "train complete"

g++ test.cpp -o test
./test modellist.txt testing_data1.txt result1.txt

echo "test complete"

g++ compute_acc.c -o compute_acc
./compute_acc

echo "compute acc complete"

cat acc.txt
