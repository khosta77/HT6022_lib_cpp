#ifndef OSCILLOSCOPES_OSCILLOSCOPE_CPP_H_
#define OSCILLOSCOPES_OSCILLOSCOPE_CPP_H_

#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <thread>
#include <chrono>
#include <exception>
#include <string>

namespace oscilloscopes
{

    constexpr int OSCILLOSCOPE_ERROR_INVALID_PARAM = 1;
    constexpr int OSCILLOSCOPE_SUCCESS             = 0;
    constexpr int OSCILLOSCOPE_ERROR_ACCESS        = -3;
    constexpr int OSCILLOSCOPE_ERROR_NO_DEVICE     = -4;
    constexpr int OSCILLOSCOPE_ERROR_TIMEOUT       = -7;
    constexpr int OSCILLOSCOPE_ERROR_NO_MEM        = -11;
    constexpr int OSCILLOSCOPE_ERROR_OTHER         = -99;

    /** @brief Ошибка матрицы, с выводом сообщения
     * */
    class OscilloscopeException : public std::exception
    {
    public:
        explicit OscilloscopeException( const std::string& msg ) : m_msg(msg) {}
        const char *what() const noexcept override
        {
            return m_msg.c_str();
        }

    private:
        std::string m_msg;
    };

    class InvalidParamOscilloscope : public OscilloscopeException
    {
    public:
        InvalidParamOscilloscope( const std::string& methodName ) :
            OscilloscopeException( ( "В методе " + methodName + " проблема с параметрами/ом" ) ) {}
    };

    class AccessOscilloscopeException : public OscilloscopeException
    {
    public:
        AccessOscilloscopeException( const std::string& methodName ) :
            OscilloscopeException( ( "В методе " + methodName + " проблема с доступом" ) ) {}
    };

    class NoDeviceOscilloscope : public OscilloscopeException
    {
    public:
        NoDeviceOscilloscope( const std::string &methodName ) :
            OscilloscopeException( ( "В методе " + methodName + " не обнаружено устройств" ) ) {}
    };

    class TimeOut : public OscilloscopeException
    {
    public:
        TimeOut( const std::string &methodName ) :
            OscilloscopeException( ( "В методе " + methodName + " произошел timeout" ) ) {}
    };

    class MemoryException : public OscilloscopeException
    {
    public:
        MemoryException( const std::string& methodName ) :
            OscilloscopeException( ( "В методе " + methodName + " под параметр нет памяти")) {}
    };

    class OtherException : public OscilloscopeException
    {
    public:
        OtherException( const std::string& methodName ) :
            OscilloscopeException( ( "В методе " + methodName + " произошло что-то неизвестное")) {}
    };

    /** @brief THROW
     * */
    template <typename T = uint8_t>
    void THROW( const int& r, const std::string& fooName, T *data = nullptr )
    {
        if( r == 0 )
            return;

        if( data != nullptr )
            delete []data;

        switch(r)
        {
            case OSCILLOSCOPE_ERROR_INVALID_PARAM:
                throw InvalidParamOscilloscope( fooName );
            case OSCILLOSCOPE_ERROR_ACCESS:
                throw AccessOscilloscopeException(fooName);
            case OSCILLOSCOPE_ERROR_NO_DEVICE:
                throw NoDeviceOscilloscope(fooName);
            case OSCILLOSCOPE_ERROR_TIMEOUT:
                throw TimeOut(fooName);
            case OSCILLOSCOPE_ERROR_NO_MEM:
                throw MemoryException(fooName);
            case OSCILLOSCOPE_ERROR_OTHER:
                throw OtherException(fooName);
            default:
                throw OscilloscopeException( "В методе " + fooName + " произошло что-то неизвестное!!!" );
        };
    }

    struct OscSignal
    {
        size_t _sampleRate;
        size_t _inputLevel;
        size_t _signalSize;
        std::vector<uint16_t> _signal;

        OscSignal() : _sampleRate(0), _inputLevel(0), _signalSize(0) {}
    };

    using OscSigframe = std::map<uint8_t, OscSignal>;

    class Oscilloscope
    {
    public:
        virtual ~Oscilloscope() {};
        virtual size_t getChannelsSize() = 0;
        virtual std::string whoAmI() = 0;

        virtual void setSampleRate( const size_t& SR ) = 0;
        virtual size_t getSampleRate() = 0;
        virtual std::vector<size_t> getRangeSampleRate() = 0;

        virtual void setInputLevel( const uint8_t& CHx, const size_t& IL ) = 0;
        virtual size_t getInputLevel( const uint8_t &CHx ) = 0;
        virtual std::vector<size_t> getRangeInputLevel() = 0;

        virtual OscSigframe getSignalFrame( const size_t& FS ) = 0;
        virtual std::vector<size_t> getRangeSignalFrame() = 0;
    };

};  // oscilloscopes

#endif  // OSCILLOSCOPES_OSCILLOSCOPE_CPP_H_
