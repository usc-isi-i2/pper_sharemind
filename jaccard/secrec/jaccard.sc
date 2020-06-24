import stdlib;
import shared3p;

domain pd_shared3p shared3p;

template <domain D>
D bool jaccard(D uint64 [[1]] a, D uint64 [[1]] b, D float32 t) {
    D bool result = true;
    D uint64 match_counter = 0;
    D float32 jaccard_result = 0;
    for (uint64 i = 0; i < size(a); i++) {
        for (uint64 j = 0; j < size(b); j++) {
            match_counter += (uint64) (a[i] == b[j]);
        }
    }
    jaccard_result = (float32)match_counter / ((float32)size(a) + (float32)size(b) - (float32)match_counter);
    result = jaccard_result >= t;
    return result;
}

void main() {
    pd_shared3p uint64 [[1]] a = argument("a");
    pd_shared3p uint64 [[1]] b = argument("b");
    pd_shared3p float32 t = argument("t");

    pd_shared3p bool result = jaccard(a, b, t);

    publish("result", result);
}
