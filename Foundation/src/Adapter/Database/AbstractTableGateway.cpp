#include "Poco/Data/SessionFactory.h"
#include "Poco/Data/MySQL/MySQLException.h"
#include "Adapter/Database/AbstractTableGateway.h"

using namespace Poco::Data::Keywords;

namespace Database {


    AbstractTableGateway::AbstractTableGateway(Poco::Data::Session & session)
        : _table(),
          _throwException(false),
          _session(session),
          _columns(),
          _columnsValues()
    {}

    Foundation::TableGatewayInterface & AbstractTableGateway::throwException(bool flag)
    {
        _throwException = flag;
        return *this;
    }

    Foundation::TableGatewayInterface & AbstractTableGateway::table(const std::string & name)
    {
        _table = name;
        return *this;
    }

    void AbstractTableGateway::removeWhere(const std::string & whereField, const std::string & whereValue)
    {
        try {
            Poco::Data::Statement remove(_session);

            /// Poco::SQL::Statement does not accept temporary variables.
            /// Therefore this copy from this parameter must be created.
            std::string localValue = whereValue;

            remove << "DELETE FROM %s WHERE %s = ?", _table, whereField, use(localValue);

            std::size_t affectedRows = remove.execute();
            if ( _throwException && affectedRows == 0 )
                throw Poco::NotFoundException("Not Found", 400);

        } catch (Poco::Data::ConnectionFailedException & exception) {
            /// @TODO Log message.
            throw Poco::Exception("Internal Server Error", 500);
        } catch (Poco::Data::MySQL::StatementException & exception) {
            /// @TODO Log message.
            throw Poco::Exception("Internal Server Error", 500);
        } catch (Poco::NotFoundException & exception) {
            exception.rethrow();
        } catch (Poco::Exception & exception) {
            /// @TODO Log message.
            throw Poco::Exception("Internal Server Error", 500);
        }
    }

    Poco::Data::RecordSet AbstractTableGateway::selectWhere(const std::string & whereField, const std::string & whereValue)
    {
        try {
            Poco::Data::Statement select(_session);

            /// Poco::Data::Statement does not accept temporary variables.
            /// Therefore these copies from these parameters must be created.
            std::string localValue = whereValue;
            std::string selectedColumns = queryColumns();

            select << "SELECT %s FROM %s WHERE %s = ?", selectedColumns, _table, whereField, use(localValue), now;

            return Poco::Data::RecordSet(select);

        } catch (Poco::Data::ConnectionFailedException & exception) {
            /// @TODO Log message.
            throw Poco::Exception("Internal Server Error", 500);
        } catch (Poco::Data::MySQL::StatementException & exception) {
            /// @TODO Log message.
            throw Poco::Exception("Internal Server Error", 500);
        } catch (Poco::Exception & exception) {
            /// @TODO Log message.
            throw Poco::Exception("Internal Server Error", 500);
        }
    }

    void AbstractTableGateway::updateWhere(const std::string & whereField, const std::string & whereValue)
    {
        std::string fields;
        if ( _columnsValues.empty() )
            throw Poco::InvalidArgumentException("Command Columns Missing", 403);

        for ( auto const & pair : _columnsValues ) {

            if ( fields.empty() ) {
                fields += pair.first + " = '" + pair.second + "'";
                continue;
            }

            fields += ", " + pair.first + " = '" + pair.second + "'";

        }

        try {
            Poco::Data::Statement update(_session);

            /// Poco::Data::Statement does not accept temporary variables.
            /// Therefore these copies from these parameters must be created.
            std::string localValue = whereValue;

            update << "UPDATE %s SET %s WHERE %s = ?", _table, fields, whereField, use(localValue);

            std::size_t affectedRows = update.execute();
            if ( _throwException && affectedRows == 0 )
                throw Poco::ApplicationException("SQL UPDATE Statement Not Performed", 500);

        } catch (Poco::ApplicationException & exception) {
            /// @TODO Log message: Poco::format("UPDATE %s SET %s WHERE %s = %s", _table, fields, whereField, localValue).
            throw Poco::Exception("Internal Server Error", 500);
        } catch (Poco::Data::ConnectionFailedException & exception) {
            /// @TODO Log message.
            throw Poco::Exception("Internal Server Error", 500);
        } catch (Poco::Data::MySQL::StatementException & exception) {
            /// @TODO Log message.
            throw Poco::Exception("Internal Server Error", 500);
        } catch (Poco::Exception & exception) {
            /// @TODO Log message.
            throw Poco::Exception("Internal Server Error", 500);
        }
    }

    void AbstractTableGateway::insert()
    {
        std::string fields;
        std::string values;
        if ( _columnsValues.empty() )
            throw Poco::InvalidArgumentException("Command Columns Missing", 403);

        for ( auto pair = _columnsValues.begin(); pair != _columnsValues.end(); ++pair ) {

            if ( fields.empty() ) {
                fields += pair->first;
                values += "'" + pair->second + "'";
                continue;
            }

            /// Let the last pair for getting collected out of the loop.
            if ( pair == --_columnsValues.end() )
                break;

            fields += ", " + pair->first;
            values += ", '" + pair->second + "'";

        }

        auto lastPair = --_columnsValues.end();

        std::string lastField = lastPair->first;
        std::string lastValue = lastPair->second;

        try {
            Poco::Data::Statement insert(_session);

            insert << "INSERT INTO %s (%s, %s) VALUES (%s, ?)", _table, fields, lastField, values, use(lastValue);

            std::size_t affectedRows = insert.execute();
            if ( _throwException && affectedRows == 0 )
                throw Poco::ApplicationException("SQL INSERT Statement Not Performed", 500);

            _columnsValues.clear();

        } catch (Poco::ApplicationException & exception) {
            /// @TODO Log message:
            /// Poco::format("INSERT INTO %s (%s, %s) VALUES (%s, %s)", _table, fields, lastField, values, lastValue)
            throw Poco::Exception("Internal Server Error", 500);
        } catch (Poco::Data::ConnectionFailedException & exception) {
            /// @TODO Log message.
            throw Poco::Exception("Internal Server Error", 500);
        } catch (Poco::Data::MySQL::StatementException & exception) {
            /// @TODO Log message.
            throw Poco::Exception("Internal Server Error", 500);
        } catch (Poco::Exception & exception) {
            /// @TODO Log message.
            throw Poco::Exception("Internal Server Error", 500);
        }
    }

    Foundation::TableGatewayInterface & AbstractTableGateway::withColumn(const std::string & name)
    {
        _columns.push_back(name);
        return *this;
    }

    Foundation::TableGatewayInterface & AbstractTableGateway::withColumn(const std::string & column, const std::string & value)
    {
        _columnsValues[column] = value;
        return *this;
    }

    std::string AbstractTableGateway::queryColumns() const
    {
        std::string columns;
        if ( _columns.empty() )
            columns = "*";

        for ( auto iterator = _columns.begin(); iterator != _columns.end(); ++iterator ) {

            columns += *iterator;
            if ( iterator != --_columns.end() )
                columns += ", ";

        }

        return columns;
    }


}
