# Sharedmind Jaccard Similarity Implementation

## Run in emulator

1. Encode record.

```
python preprocessing.py <record1> <record2> <ngram size>

Example:
$ python preprocessing.py "hello" "helle" 3
a: {'ell', 'llo', 'hel'}
b: {'lle', 'ell', 'hel'}
jaccard similarity score: 0.5
encoded a: [0x6c6c6f, 0x68656c, 0x656c6c]
encoded b: [0x6c6c65, 0x68656c, 0x656c6c]
```

Each character is encoded into corresponding ASCII number (8 bits), so the ngram size can be from 1 to 8 (uint64 size limit).

2. Generate emulator input.

```
$ python ../argument-stream-cipher.py \
a pd_share3p uint64 "[0x6c6c6f, 0x68656c, 0x656c6c]" \
b pd_share3p uint64 "[0x6c6c65, 0x68656c, 0x656c6c]" \
t pd_share3p float32 0.4 > input.bin
```

3. Compile and run Jaccard in emulator.

```
# compile
$ scc -o secrec/jaccard.sb -I/root/build-sdk/secrec-stdlib secrec/jaccard.sc
# run
$ sharemind-emulator secrec/jaccard.sb --cfile=input.bin > result.bin
```

4. Decipher result.

```
$ cat result.bin | python ../argument-stream-decipher.py
result = [True]
```

## Run in VM

Make sure VM license is installed. This `jaccard` folder should be located at `~/Sharemind-SDK/jaccard`.

```
# compile SecreC program
sm_compile.sh secrec/*.sc

# compile client program
mkdir build
cd build
cmake ..
make

# start server and run
sm_start_servers.sh
./jaccard --a 0x6c6c6f 0x68656c 0x656c6c --b 0x6c6c65 0x68656c 0x656c6c --t 0.4
```
