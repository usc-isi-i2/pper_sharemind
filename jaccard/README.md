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

## Run End-to-end program

0. On server side, configure keydb. The keydb name has to be "dbhost". On client side, compile all SecreC and client programs.

```
[Host host]
; The name to access this host from the SecreC application.
Name = dbhost
```

1. On each client, make a CSV file which has three columns:

```
id,original_id,tokens
ds1_0,rec-242-org,0x656e 0x6e67 0x6720 0x2066
ds1_1,rec-160-dup-2,0x6672 0x7265 0x6520 0x2066 0x6620 0x2033 0x3331 0x3120
...
```

Then upload data to keydb on each client. 

```
tail -n +2 <input-csv-file.csv> | awk -F',' '{print "build/upload --key "$1" --tokens "$3}' | xargs -I {} sh -c "{}"
```

2. Compute and find pairs from one of the clients.

```
build/link --a_prefix ds1_ --a_size=2 --b_prefix ds2_ b_size=8 --t 0.5
```

