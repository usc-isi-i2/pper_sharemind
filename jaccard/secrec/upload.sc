import stdlib;
import shared3p;
import keydb;
import shared3p_keydb;

domain pd_shared3p shared3p;

void main() {
    keydb_connect("dbhost");

    string key = argument("key");
    pd_shared3p uint64 [[1]] tokens = argument("tokens");

    keydb_set(key, tokens);

    keydb_disconnect();
}
