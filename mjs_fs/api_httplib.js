let HTTPLib = {
    _send: ffi('void* http_send(void*)'),

    send: function (req) {
        let obj = Object.create(HTTPRes._proto);
        obj.ins = HTTPLib._send(req.ins);
        return obj;
    }
};
let HTTPRes = {
    _proto: {
        free: function () {
            ffi('void http_free_res(void *)')(this.ins);
        },
        getStatus: function () {
            return ffi('int HTTPRes_getStatus(void *)')(this.ins);
        }
    }
};
let HTTPReq = {
    _crt: ffi('void* http_create_req(char *, int, char*)'),

    create: function (url, mthd, ct) {
        let obj = Object.create(HTTPReq._proto);
        obj.ins = HTTPReq._crt(url, mthd, ct);
        return obj;
    },
    _proto: {
        getURL: function () {
            return ffi('char* HTTPReq_getURL(void *)')(this.ins);
        },
        addHeader: function (name, val) {
            ffi('void http_req_add_header(void *, char*, char*)')(this.ins, name, val);
        },
        getHeader: function () {
            return ffi('char* HTTPReq_getHeader(void *)')(this.ins);
        },
        getMethod: function () {
            return ffi('int HTTPReq_getMethod(void *)')(this.ins);
        },
        getRawBody: function () {
            return ffi('char* HTTPReq_getRawBody(void *)')(this.ins);
        },
        addFormInt: function (name, val) {
            return ffi('void http_add_form_int(void *, char*, int)')(this.ins, name, val);
        },
        addFormStr: function (name, val) {
            return ffi('void http_add_form_str(void *, char*, char*)')(this.ins, name, val);
        },
        addFormFloat: function (name, val) {
            return ffi('void http_add_form_float(void *, char*, float)')(this.ins, name, val);
        },
        setBody: function (body) {
            return ffi('void http_set_request_body(void *, char*)')(this.ins, body);
        }
    }
};
