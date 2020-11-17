import stdlib;
import keydb;
import shared3p;
import shared3p_keydb;

domain pd_shared3p shared3p;

void scan(string pattern) {
    ScanCursor sc = keydb_scan(pattern);
    while(sc.cursor != 0) {
        print("Found key:", sc.key);
        sc = keydb_scan_next(sc);
    }
}

void delete(string pattern) {
    ScanCursor sc = keydb_scan(pattern);
    while(sc.cursor != 0) {
        print("Deleting key: ", sc.key);
        keydb_del(sc.key);
        sc = keydb_scan_next(sc);
    }
}

void main() {
    keydb_connect("dbhost");

    string op = argument("op");

    if (op == "scan") {
        string pattern = argument("pattern");
        scan(pattern);
    }
    if (op == "delete") {
        string pattern = argument("pattern");
        delete(pattern);
    }
    if (op == "get") {
        // string key = argument("key");
        // string type_ = argument("type");
        // bool private = argument("private");
        // bool array = argument("array");
        // get_value(key, type_, private, array);
    }

    keydb_disconnect();
}
