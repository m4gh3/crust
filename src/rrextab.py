import sortedcontainers

rrex_main_tree = sortedcontainers.SortedDict()

DO_CALLBACK  = 0x80000000
DO_RECURSION = 0x40000000
REDMASK      = 0x3fffffff

class rrex_data:
    def __init__(self):
        self.next_ref = sortedcontainers.SortedDict()
        self.reduce = -1
    def __repr__(self):
        return "{0}( reduce: {1}, next_ref: {2} )".format(type(self).__name__, self.reduce, self.next_ref )

def rrex_insert(key_vec, reduce ):
    next_ref = rrex_main_tree
    for key in key_vec[0:-1]:
        key = (key[1], -key[0])
        if not (key in next_ref):
            next_ref[key] = rrex_data()
            #next_ref[key]["reduce"] = -1
        next_ref = next_ref[key].next_ref
    key = key_vec[-1]
    if (not key in next_ref):
        next_ref[key] = rrex_data
    next_ref[key].reduce = reduce

class match_shared:
    def __init__(self, ret, root, buf, redbuf, input_func, output_func, offset, callback ):
        self.ret = ret
        self.root = root
        self.buf = buf
        self.redbuf = redbuf
        self.input_func = input_func
        self.output_func = output_func
        self.offset = offset
        self.callback = callback

def match(m, next_ref, idx ):
    matching = True
    while matching:
        matching = False
        if m.redbuf.size() > m.offset:
            c = m.redbuf.pop()
        else:
            if( idx < m.offset ):
                c = m.redbuf[idx]
            elif idx-m.offset == m.buf.size():
                #if m.buf.is_full:
                #    sys.exit(1)
                m.buf.append(m.input.get());
                c = m.buf[-1]
            else:
                c = m.buf[idx-m.offset]
            idx += 1

        to_check = 0
        #we give up lower bound
        for key in next_ref.keys():
            if (c,c) < key:
                to_check += 1
        for i in range(to_check):
            it = next_ref.keys()[i]
            if (c,c) < it:
                if len(next_ref[it].next_ref) != 0:
                    if( to_check == 1 and next_ref[it].reduce < 0 ):
                        next_ref = next_ref[it].next_ref
                        matching = True
                    match(m, next_ref[it].next_ref, idx )
                if next_ref[it].reduce >= 0:
                    if( idx > m.ret[0] ):
                        m.ret[0] = idx
                        m.ret[1] = next_ref[it].next_ref
                    m.redbuf.append(next_ref[it].reduce)
                    if to_check == 1:
                        next_ref = m.root
                        matching = True
                    match(m, m.root, idx )
                to_check -= 1

def process_input(root, ret, buf, redbuf, lval, input_func, output_func, callback, idx ):

    rval = None
    last_good = -1
    next_redbuf = []
    m = match_shared(ret, root, buf, redbuf, input_func, output_func, 0, callback )
    keep_processing = True
    while keep_processing:
        ret[0] = 0
        ret[1] = -1
        m.offset = len(redbuf)
        match(m, root, idx )

        if ret[1] >= 0:
            last_good = ret[1]
            redbuf.append(last_good & REDMASK )
            if( last_good & DO_CALLBACK ):
                rval = callback(last_good & REDMASK, m, next_redbuf, lval, rval )
            buf = buf[m.ret[0]-m.offset:len(buf)]
            ret[0] = 0
            ret[1] = 1
            if( last_good & DO_RECURSION ):
                next_redbuf.append(last_good & REDMASK )
                rval = process_input(root, ret, buf, next_redbuf, rval, input_func, output_func, callback, 0 )
                redbuf.append(ret[1])
        else:
            keep_processing = False

    ret[1] = last_good & REDMASK
    return lval
