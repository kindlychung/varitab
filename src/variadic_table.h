#ifndef VARIADICTABLE_H
#define VARIADICTABLE_H

#include <cassert>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <ios>
#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

enum class VariadicTableColumnFormat { AUTO, SCIENTIFIC, FIXED, PERCENT };

typedef decltype(&std::right) right_type;
typedef decltype(&std::left) left_type;
typedef decltype(std::setw(0)) setw_type;

setw_type setww(int w, std::wstring s);

template <typename StreamType>
void print_horizontal_line(StreamType &stream, int total_width);

setw_type setww(int w, std::wstring s) {
    auto delta = wcswidth(s.c_str(), s.length()) - s.length();
    return std::setw(w - delta);
}

template <typename StreamType>
void print_horizontal_line(StreamType &stream, int total_width) {
    // Print out the top line
    for (int i = 0; i < total_width; i++) {
        stream << L"\u2014";
    }
    stream << L"\n";
    return;
}

/**
 * A class for "pretty printing" a table of data.
 *
 * Requries C++11 (and nothing more)
 *
 * It's templated on the types that will be in each column
 * (all values in a column must have the same type)
 *
 * For instance, to use it with data that looks like:  "Fred", 193.4, 35, "Sam"
 * with header names: "Name", "Weight", "Age", "Brother"
 *
 * You would invoke the table like so:
 * VariadicTable<std::string, double, int, std::string> vt({"Name", "Weight",
 * "Age", "Brother"});
 *
 * Then add the data to the table:
 * vt.addRow({"Fred", 193.4, 35, "Sam"});
 *
 * And finally print it:
 * vt.print();
 */
template <class... Ts>
class VariadicTableWide {
   public:
    /// The type stored for each row
    typedef std::tuple<Ts...> DataTuple;

    template <typename StreamType>
    StreamType &_print_row(StreamType &stream, std::wstring val, int w) {
        stream << std::wstring(_cell_padding, L' ') << setww(w, val)
               << justify<decltype(val)>(0) << val
               << std::wstring(_cell_padding, L' ') << L"┋";
        return stream;
    }

    template <typename T, typename StreamType>
    StreamType &_print_row(StreamType &stream, T val, int w) {
        stream << std::wstring(_cell_padding, L' ') << std::setw(w)
               << justify<decltype(val)>(0) << val
               << std::wstring(_cell_padding, L' ') << L"┋";
        return stream;
    }

    /**
     * Construct the table with headers
     *
     * @param headers The names of the columns
     * @param static_column_size The size of columns that can't be found
     * automatically
     */
    VariadicTableWide(std::vector<std::wstring> headers,
                      unsigned int static_column_size = 0,
                      unsigned int cell_padding = 1)
        : _headers(headers),
          _num_columns(std::tuple_size<DataTuple>::value),
          _static_column_size(static_column_size),
          _cell_padding(cell_padding) {
        assert(headers.size() == _num_columns);
    }

    /**
     * Add a row of data
     *
     * Easiest to use like:
     * table.addRow({data1, data2, data3});
     *
     * @param data A Tuple of data to add
     */
    void addRow(std::tuple<Ts...> data) { _data.push_back(data); }

    /**
     * Pretty print the table of data
     */
    template <typename StreamType>
    void print(StreamType &stream) {
        size_columns();

        // Start computing the total width
        // First - we will have _num_columns + 1 "┋" characters
        unsigned int total_width = _num_columns + 1;

        // Now add in the size of each colum
        for (auto &col_size : _column_sizes)
            total_width += col_size + (2 * _cell_padding);

        print_horizontal_line(stream, total_width);

        // Print out the headers
        stream << L"┋";
        for (unsigned int i = 0; i < _num_columns; i++) {
            // Must find the center of the column
            //   int header_width = wcswidth(_headers[i].c_str(),
            //   _headers[i].length()); auto half = _column_sizes[i] / 2; half
            //   -= header_width / 2; std::wcout << "half: " << half <<
            //   std::endl;

            stream << std::wstring(_cell_padding, L' ')
                   << setww(_column_sizes[i], _headers[i]) << std::left
                   << _headers[i] << std::wstring(_cell_padding, L' ') << L"┋";
        }

        stream << L"\n";

        // Print out the line below the header
        print_horizontal_line(stream, total_width);

        // Now print the rows of the table
        for (auto &row : _data) {
            stream << L"┋";
            print_each(row, stream);
            stream << L"\n";
        }

        // Print out the line below the header
        print_horizontal_line(stream, total_width);
    }

    /**
     * Set how to format numbers for each column
     *
     * Note: this is ignored for std::string columns
     *
     * @column_format The format for each column: MUST be the same length as the
     * number of columns.
     */
    void setColumnFormat(
        const std::vector<VariadicTableColumnFormat> &column_format) {
        assert(column_format.size() == std::tuple_size<DataTuple>::value);

        _column_format = column_format;
    }

    /**
     * Set how many digits of precision to show for floating point numbers
     *
     * Note: this is ignored for std::string columns
     *
     * @column_format The precision for each column: MUST be the same length as
     * the number of columns.
     */
    void setColumnPrecision(const std::vector<int> &precision) {
        assert(precision.size() == std::tuple_size<DataTuple>::value);
        _precision = precision;
    }

   protected:
    // Attempts to figure out the correct justification for the data
    // If it's a floating point value
    template <typename T,
              typename = typename std::enable_if<std::is_arithmetic<
                  typename std::remove_reference<T>::type>::value>::type>
    static right_type justify(int /*firstchoice*/) {
        return std::right;
    }

    // Otherwise
    template <typename T>
    static left_type justify(long /*secondchoice*/) {
        return std::left;
    }

    /**
     * These three functions print out each item in a Tuple into the table
     *
     * Original Idea From From https://stackoverflow.com/a/26908596
     *
     * BTW: This would all be a lot easier with generic lambdas
     * there would only need to be one of this sequence and then
     * you could pass in a generic lambda.  Unfortunately, that's C++14
     */

    /**
     *  This ends the recursion
     */
    template <typename TupleType, typename StreamType>
    void print_each(TupleType &&, StreamType & /*stream*/,
                    std::integral_constant<
                        size_t, std::tuple_size<typename std::remove_reference<
                                    TupleType>::type>::value>) {}

    /**
     * This gets called on each item
     */
    template <std::size_t I, typename TupleType, typename StreamType,
              typename = typename std::enable_if<
                  I != std::tuple_size<typename std::remove_reference<
                           TupleType>::type>::value>::type>
    void print_each(TupleType &&t, StreamType &stream,
                    std::integral_constant<size_t, I>) {
        auto &val = std::get<I>(t);

        // Set the precision
        if (!_precision.empty()) {
            assert(_precision.size() ==
                   std::tuple_size<
                       typename std::remove_reference<TupleType>::type>::value);

            stream << std::setprecision(_precision[I]);
        }

        // Set the format
        if (!_column_format.empty()) {
            assert(_column_format.size() ==
                   std::tuple_size<
                       typename std::remove_reference<TupleType>::type>::value);

            if (_column_format[I] == VariadicTableColumnFormat::SCIENTIFIC)
                stream << std::scientific;

            else if (_column_format[I] == VariadicTableColumnFormat::FIXED)
                stream << std::fixed;

            else if (_column_format[I] == VariadicTableColumnFormat::PERCENT)
                stream << std::fixed << std::setprecision(2);
        }

        _print_row(stream, val, _column_sizes[I]);
        // stream << std::string(_cell_padding, ' ') <<
        // std::setw(_column_sizes[I])
        //        << justify<decltype(val)>(0) << val <<
        //        std::string(_cell_padding, ' ') << "┋";

        // Unset the format
        if (!_column_format.empty()) {
            // Because "stream << std::defaultfloat;" won't compile with old GCC
            // or Clang
            stream.unsetf(std::ios_base::floatfield);
        }

        // Recursive call to print the next item
        print_each(std::forward<TupleType>(t), stream,
                   std::integral_constant<size_t, I + 1>());
    }

    /**
     * his is what gets called first
     */
    template <typename TupleType, typename StreamType>
    void print_each(TupleType &&t, StreamType &stream) {
        print_each(std::forward<TupleType>(t), stream,
                   std::integral_constant<size_t, 0>());
    }

    /**
     * Try to find the size the column will take up
     *
     * If the datatype has a size() member... let's call it
     */
    size_t sizeOfData(std::wstring const &data) {
        return wcswidth(data.c_str(), data.length());
    }

    template <class T>
    size_t sizeOfData(const T &data,
                      decltype(((T *)nullptr)->size()) * /*dummy*/ = nullptr) {
        return data.size();
    }

    /**
     * Try to find the size the column will take up
     *
     * If the datatype is an integer - let's get it's length
     */
    template <class T>
    size_t sizeOfData(
        const T &data,
        typename std::enable_if<std::is_integral<T>::value>::type * /*dummy*/ =
            nullptr) {
        if (data == 0) return 2;
        int n_digits = std::log10(abs(data));
        int n_commas = n_digits / 3;
        return n_digits + n_commas + 2;
    }

    /**
     * Try to find the size the column will take up
     *
     * If the datatype is an floating point - let's get it's length
     */
    template <class T>
    size_t sizeOfData(
        const T &data,
        typename std::enable_if<std::is_floating_point<T>::value>::type
            * /*dummy*/
        = nullptr) {
        auto data_abs = fabs(data);
        auto data_log10 = std::ceil(std::log10(data_abs));
        return data_log10 + 1 + 4 + 5;  // 100.001, 3 decimal places
    }

    /**
     * If it doesn't... let's just use a statically set size
     */
    size_t sizeOfData(...) { return _static_column_size; }

    /**
     * These three functions iterate over the Tuple, find the printed size of
     * each element and set it in a vector
     */

    /**
     * End the recursion
     */
    template <typename TupleType>
    void size_each(TupleType &&, std::vector<unsigned int> & /*sizes*/,
                   std::integral_constant<
                       size_t, std::tuple_size<typename std::remove_reference<
                                   TupleType>::type>::value>) {}

    /**
     * Recursively called for each element
     */
    template <std::size_t I, typename TupleType,
              typename = typename std::enable_if<
                  I != std::tuple_size<typename std::remove_reference<
                           TupleType>::type>::value>::type>
    void size_each(TupleType &&t, std::vector<unsigned int> &sizes,
                   std::integral_constant<size_t, I>) {
        auto elem = std::get<I>(t);
        sizes[I] = sizeOfData(elem);
        // std::wcout << L"col " << I << ": " << elem;
        // std::wcout << L", size: " << sizes[I];
        // std::wcout << L"\n";

        // Override for Percent
        if (!_column_format.empty())
            if (_column_format[I] == VariadicTableColumnFormat::PERCENT)
                sizes[I] = 6;  // 100.00

        // Continue the recursion
        size_each(std::forward<TupleType>(t), sizes,
                  std::integral_constant<size_t, I + 1>());
    }

    /**
     * The function that is actually called that starts the recursion
     */
    template <typename TupleType>
    void size_each(TupleType &&t, std::vector<unsigned int> &sizes) {
        size_each(std::forward<TupleType>(t), sizes,
                  std::integral_constant<size_t, 0>());
    }

    /**
     * Finds the size each column should be and set it in _column_sizes
     */
    void size_columns() {
        _column_sizes.resize(_num_columns);
        // Temporary for querying each row
        std::vector<unsigned int> column_sizes(_num_columns);
        // Start with the size of the headers
        for (unsigned int i = 0; i < _num_columns; i++) {
            int w = wcswidth(_headers[i].c_str(), _headers[i].length());
            //   std::wcout << _headers[i] << ", width: " << w << std::endl;
            _column_sizes[i] = w;
        }
        // Grab the size of each entry of each row and see if it's bigger
        for (auto &row : _data) {
            size_each(row, column_sizes);
            for (unsigned int i = 0; i < _num_columns; i++)
                _column_sizes[i] = std::max(_column_sizes[i], column_sizes[i]);
        }
    }

    /// The column headers
    std::vector<std::wstring> _headers;

    /// Number of columns in the table
    unsigned int _num_columns;

    /// Size of columns that we can't get the size of
    unsigned int _static_column_size;

    /// Size of the cell padding
    unsigned int _cell_padding;

    /// The actual data
    std::vector<DataTuple> _data;

    /// Holds the printable width of each column
    std::vector<unsigned int> _column_sizes;

    /// Column Format
    std::vector<VariadicTableColumnFormat> _column_format;

    /// Precision For each column
    std::vector<int> _precision;
};

#endif
