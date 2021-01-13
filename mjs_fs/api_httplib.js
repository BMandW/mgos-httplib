let HTTPLib = {
    MT_GET: 1,
    MT_POST: 2,
    CT_FORM: 'application/x-www-form-urlencoded',
    CT_JSON: 'application/json',

    _send: ffi('void* http_send(void*)'),
    _strlen: ffi('int strlen(void*)'),

    send: function (req) {
        let obj = Object.create(HTTPRes._proto);
        obj.ins = HTTPLib._send(req.ins);
        return obj;
    }
};
let HTTPRes = {
    _proto: {
        free: function () {
            ffi('void http_res_free(void *)')(this.ins);
        },
        getStatus: function () {
            return ffi('int HTTPRes_getStatus(void *)')(this.ins);
        },
        getBody: function () {
            return ffi('char* HTTPRes_getBody(void *)')(this.ins);
        },
        isSuccess: function () {
            return ffi('bool HTTPRes_isSuccess(void *)')(this.ins);
        },
        getCLen: function () {
            return ffi('int HTTPRes_getContentLength(void *)')(this.ins);
        },
        getHeader: function () {
            return ffi('char* HTTPRes_getHeader(void *)')(this.ins);
        },
        getHeaderVal: function (name) {
            let b = ffi('void* http_res_header_value(void*, char*)')(this.ins, name);
            if (b !== null) {
                let l = HTTPLib._strlen(b);
                let s = mkstr(b, 0, l, true);
                return s;
            } else {
                return null;
            }
        },
        getHeaderValWithBuff: function (name, buff) {
            let b = ffi('char* http_res_hval_buff(void*, char*, void*, int)')(this.ins, name, buff, 256);
            if (b !== null) {
                let l = HTTPLib._strlen(buff);
                let s = mkstr(buff, 0, l, true);
                return s;
            } else {
                return null;
            }
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
        addFormVal: function (name, val) {
            return ffi('void http_add_form_val(void *, char*, char*)')(this.ins, name, val);
        },
        setBody: function (body) {
            return ffi('void http_set_request_body(void *, char*)')(this.ins, body);
        }
    }
};
