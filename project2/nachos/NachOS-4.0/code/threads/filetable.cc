#include "filetable.h"

FileTable::FileTable() {
    for (int i = 0; i < MAX_FILE; ++i) {
        _files[i] = NULL;
        _type[i] = -1;
        _curOffset[i] = 0;
    }
}

FileTable::~FileTable() {
    for (int i = 0; i < MAX_FILE; ++i) {
        if (_type[i] == -1) continue;
        delete _files[i];
        _files[i] = NULL;
        _type[i] = -1;
        _curOffset[i] = 0;
    }
}

int FileTable::Open(OpenFile* pFile, int type) {
    if (pFile == NULL) return -1;
    if (type != 0 && type != 1) return -1;
    for (int i = 0; i < MAX_FILE; ++i) {
        if (_type[i] != -1) continue;
        _files[i] = pFile;
        _type[i] = type;
        _curOffset[i] = 0;
        return i;
    }
    return -1;
}

int FileTable::Close(int fid) {
    if (fid < 0 || fid >= MAX_FILE) return -1;
    if (_type[fid] == -1) return -1;
    delete _files[fid];
    _files[fid] = NULL;
    _type[fid] = -1;
    _curOffset[fid] = 0;
    return 0;
}

int FileTable::GetType(int fid) {
    if (fid < 0 || fid >= MAX_FILE) return -1;
    return _type[fid];
}

int FileTable::ReadChar(char &c, int fid) {
    if (fid < 0 || fid >= MAX_FILE) return -1;
    if (_type[fid] == -1) return -1;
    int ret = _files[fid]->ReadAt(&c, 1, _curOffset[fid]);
    _curOffset[fid] += ret;
    return ret;
}

int FileTable::WriteChar(char &c, int fid) {
    if (fid < 0 || fid >= MAX_FILE) return -1;
    if (_type[fid] != 0) return -1;
    int ret = _files[fid]->WriteAt(&c, 1, _curOffset[fid]);
    _curOffset[fid] += ret;
    return ret;
}

int FileTable::Seek(int pos, int fid) {
    if (fid < 0 || fid >= MAX_FILE) return -1;
    if (_type[fid] == -1) return -1;
    int len = _files[fid]->Length();
    if (pos == -1) {
        _curOffset[fid] = len-1;
        return len-1;
    } else if (pos >= 0 && pos < len) {
        _curOffset[fid] = pos;
        return pos;
    } else return -1;
}
