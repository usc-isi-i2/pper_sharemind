import sys
import csv
import argparse
import preprocessing as pp

if __name__ == '__main__':
    '''
    python preprocessing_febrl.py test_data/gen-1k_300-700-5-5-5-zipf-all_200.csv test_data/ds1_output.csv \
        --prefix=ds1_ --ngram=2 --blocking --num-perm=128 --threshold=0.5
    '''

    parser = argparse.ArgumentParser(description='Preprocess Febrl dataset')
    parser.add_argument('infile', nargs='?', type=argparse.FileType('r'), default=sys.stdin)
    parser.add_argument('outfile', nargs='?', type=argparse.FileType('w'), default=sys.stdout)
    parser.add_argument('--prefix', dest='prefix', action='store', required=True)
    parser.add_argument('--ngram', dest='ngram', action='store', type=int, default=2)
    parser.add_argument('--blocking', dest='blocking', action='store_true')
    parser.add_argument('--num-perm', dest='num_perm', action='store', type=int, default=128)
    parser.add_argument('--threshold', dest='threshold', action='store', type=float)
    args = parser.parse_args()

    fieldnames = ['id', 'original_id', 'tokens'] + ['blocking_keys'] if args.blocking else []
    csv_in = csv.DictReader(args.infile)
    csv_out = csv.DictWriter(args.outfile, fieldnames=fieldnames)
    csv_out.writeheader()
    for idx, line in enumerate(csv_in):
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
        value = pp.ngram(args.ngram, value)
        value = list(filter(lambda x: x != '', value))

        output = {
            'id': args.prefix + str(idx),
            'original_id': line['rec_id'],
            'tokens': ' '.join([hex(pp.encode_token(t)) for t in value])
        }

        if args.blocking:
            blocking_keys = pp.generate_blocking_keys(value, num_perm=args.num_perm, threshold=args.threshold)
            output['blocking_keys'] = ' '.join(blocking_keys)  # base64 has no space

        csv_out.writerow(output)



