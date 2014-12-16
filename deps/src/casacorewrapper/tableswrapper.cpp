#include <casa/Containers/ValueHolder.h>
#include <tables/Tables/TableProxy.h>
#include <tables/Tables/TableColumn.h>
#include <tables/Tables/ScaColData.h>
#include <tables/Tables/ArrColData.h>

using namespace casa;

extern "C" {
    TableProxy* newTable(char* name, char* endianFormat, char* memType, int nrow) {
        return new TableProxy(String(name),Record(),endianFormat,memType,nrow,Record(),Record());
    }
    TableProxy* newTable_existing(char* name, int option) {
        return new TableProxy(String(name),Record(),option);
    }
    void deleteTable(TableProxy* t) {delete t;}

    void flush(TableProxy* t, bool recursive) {t->flush(recursive);}

    bool isReadable(TableProxy const* t) {return t->isReadable();}
    bool isWritable(TableProxy const* t) {return t->isWritable();}

    int nrows(TableProxy* t) {return t->nrows();}
    int ncolumns(TableProxy* t) {return t->ncolumns();}

    void addRow(TableProxy* t, int nrows) {t->addRow(nrows);}

    bool canRemoveRow(TableProxy* t) {return t->table().canRemoveRow();}
    void removeRow(TableProxy* t, int* rownrs, size_t nrows) {
        Vector<Int> rows(Block<Int>(nrows,rownrs,false));
        t->removeRow(rows);
    }

    void addScalarColumn(TableProxy* t, char* name, int type) {
        switch(type) {
        case TpInt:
            ScalarColumnDesc<Int> column(name);
            t->table().addColumn(column);
            break;
        }
    }

    void addArrayColumn(TableProxy* t, char* name, int type, int* dim, size_t ndim) {
        IPosition dimensions(ndim);
        for (uint i = 0; i < ndim; ++i) {
            dimensions[i] = dim[i];
        }
        switch (type) {
        case TpInt:
            ArrayColumnDesc<Int> column(name,"",dimensions);
            t->table().addColumn(column);
            break;
        }
    }

    void removeColumn(TableProxy* t, char* name) {
        Vector<String> column(1);
        column[0] = String(name);
        t->removeColumns(column);
    }

    unsigned int nKeywords(TableProxy* t) {
        Record record = t->getKeywordSet(String());
        return record.nfields();
    }

    void getKeyword_string(TableProxy* t, char* keyword, char* output, size_t outputlength) {
        ValueHolder value = t->getKeyword(String(),String(keyword),-1);
        String str = value.asString();
        for (uint i = 0; i < str.length() && i < outputlength; ++i) {
            output[i] = str[i];
        }
        output[str.length()] = '\0';
    }

    void getColumnType(TableProxy* t, char* column, char* output, size_t outputlength) {
        String typestr = t->columnDataType(String(column));
        for (uint i = 0; i < typestr.length() && i < outputlength; ++i) {
            output[i] = typestr[i];
        }
        output[typestr.length()] = '\0';
    }

    void getColumnShape(TableProxy* t, char* column, int* output, size_t outputlength) {
        if (t->isScalarColumn(String(column))) {
            if (outputlength > 1) {
                output[0] = nrows(t);
                output[1] = -1;
            }
        }
        else {
            // Possibly need to call syncTable(t) here.
            ROTableColumn col(t->table(),String(column));
            // Assume the number of dimensions and shape of the first cell
            // is representative of the entire column.
            uint ndim = col.ndim(0);
            IPosition shape = col.shape(0);
            for (uint i = 0; i < ndim && i < outputlength; ++i) {
                output[i] = shape[i];
            }
            if (outputlength > ndim+1) {
                output[ndim]   = nrows(t);
                output[ndim+1] = -1;
            }
        }
    }

    void getColumn_int(TableProxy* t, char* column, int* output, size_t outputlength) {
        ValueHolder value = t->getColumn(String(column),0,-1,1);
        Array<Int> arr = value.asArrayInt();
        int idx = 0;
        for (Array<Int>::IteratorSTL it = arr.begin(); it != arr.end(); ++it) {
            output[idx] = *it;
            ++idx;
        }
    }

    void getColumn_double(TableProxy* t, char* column, double* output, size_t outputlength) {
        ValueHolder value = t->getColumn(String(column),0,-1,1);
        Array<Double> arr = value.asArrayDouble();
        int idx = 0;
        for (Array<Double>::IteratorSTL it = arr.begin(); it != arr.end(); ++it) {
            output[idx] = *it;
            ++idx;
        }
    }

    void getColumn_complex(TableProxy* t, char* column, std::complex<float>* output, size_t outputlength) {
        ValueHolder value = t->getColumn(String(column),0,-1,1);
        Array<Complex> arr = value.asArrayComplex();
        int idx = 0;
        for (Array<Complex>::IteratorSTL it = arr.begin(); it != arr.end(); ++it) {
            output[idx] = *it;
            ++idx;
        }
    }

    void putColumn_complex(TableProxy* t, char* column, std::complex<float>* input,
                           size_t* shape, size_t ndim) {
        IPosition dimensions(ndim);
        for (uint i = 0; i < ndim; ++i)
            dimensions[i] = shape[i];
        Array<Complex> arr(dimensions);
        int idx = 0;
        for (Array<Complex>::IteratorSTL it = arr.begin(); it != arr.end(); ++it) {
            *it = input[idx];
            ++idx;
        }
        ValueHolder value(arr);
        t->putColumn(String(column),0,-1,1,value);
    }
}
