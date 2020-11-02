import csv
import sys
ds1_path = '/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/test_data/gen-1k_300-700-5-5-5-zipf-all_200.csv'
ds2_path = '/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/test_data/gen-1k_300-700-5-5-5-zipf-all_800.csv'
ds1 = {}
ds2 = {}
t = 0.5
N = int(sys.argv[1])


def jaccard(a, b, t):
    return (1.0 * (len(a & b) / len(a | b))) >= t

def ngram(n, s, place_holder=' ', padded=False):
    if len(s) == 0:
        return []
    if padded:
        pad = place_holder * (n - 1)
        s = pad + s + pad
    s = s.split(' ')
    s = place_holder.join(s)
    if len(s) < n:
        return [s]
    return [s[i:i + n] for i in range(len(s) - n + 1)]



def processing():
    with open(ds1_path, 'r') as f1:
        csvfile = csv.DictReader(f1)
        for idx, line in enumerate(csvfile):
            if idx >= N:
                break
            value = line['culture']\
                + ' ' + line['sex']\
                + ' ' + line['age']\
                + ' ' + line['date_of_birth']\
                + ' ' + line['title']\
                + ' ' + line['given_name']\
                + ' ' + line['surname']\
                + ' ' + line['state']\
                + ' ' + line['suburb']\
                + ' ' + line['postcode']\
                + ' ' + line['street_number']\
                + ' ' + line['address_1']\
                + ' ' + line['address_2']\
                + ' ' + line['phone_number']\
                + ' ' + line['soc_sec_id']
            value = value.lower().strip()
            value = ngram(2, value)
            value = list(filter(lambda x: x != '', value))
            value = [ord(elem) for sub in value for elem in sub]
            ds1[line['rec_id']] = value
    with open(ds2_path, 'r') as f2:
        csvfile = csv.DictReader(f2)
        for idx, line in enumerate(csvfile):
            if idx >= N:
                break
            value = line['culture']\
                + ' ' + line['sex']\
                + ' ' + line['age']\
                + ' ' + line['date_of_birth']\
                + ' ' + line['title']\
                + ' ' + line['given_name']\
                + ' ' + line['surname']\
                + ' ' + line['state']\
                + ' ' + line['suburb']\
                + ' ' + line['postcode']\
                + ' ' + line['street_number']\
                + ' ' + line['address_1']\
                + ' ' + line['address_2']\
                + ' ' + line['phone_number']\
                + ' ' + line['soc_sec_id']
            value = value.lower().strip()
            value = ngram(2, value)
            value = list(filter(lambda x: x != '', value))
            value = [ord(elem) for sub in value for elem in sub]
            # value = list(filter(lambda x: x != '__', value))
            ds2[line['rec_id']] = value
        with open('/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/test_data/tokenized_set_A.csv', 'w+') as file1:
            file1.write("%s:%s\n"%("key", "tokens"))
            for key in ds1.keys():
                file1.write("%s:%s\n"%(key, ds1[key]))
        with open('/Users/tanmay.ghai/Desktop/palisade-development/src/pke/examples/test_data/tokenized_set_B.csv', 'w+') as file2:
            file2.write("%s:%s\n"%("key", "tokens"))
            for key in ds2.keys():
                file2.write("%s:%s\n"%(key, ds2[key]))

processing()
