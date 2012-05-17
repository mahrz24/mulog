#ifndef _MULOG_H_
#define _MULOG_H_
#include <list>
#include <vector>
#include <functional>
#include <string>
#include <memory>
#include <sstream>
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <regex>

#define BLOCK_BEGIN (*mulog::core::get().dispatch_log_message(\
          mulog::log_header({mulog::info, mulog::type::block_begin, \
          __PRETTY_FUNCTION__,__FILE__,__LINE__,"__block_begin"})))
#define BLOCK_END (*mulog::core::get().dispatch_log_message(\
          mulog::log_header({mulog::info, mulog::type::block_end, \
          __PRETTY_FUNCTION__,__FILE__,__LINE__,"__block_end"})))
#define HEADER (*mulog::core::get().dispatch_log_message(\
          mulog::log_header({mulog::info, mulog::type::header, \
          __PRETTY_FUNCTION__,__FILE__,__LINE__,"__header"})))
#define INFO (*mulog::core::get().dispatch_log_message(\
          mulog::log_header({mulog::info, mulog::type::log, \
          __PRETTY_FUNCTION__,__FILE__,__LINE__,""})))
#define LOG(s) (*mulog::core::get().dispatch_log_message(\
          mulog::log_header({mulog::s, mulog::type::log, \
          __PRETTY_FUNCTION__,__FILE__,__LINE__,""})))
#define LOG_TAG(s,t) (*mulog::core::get().dispatch_log_message(\
          mulog::log_header({mulog::s, mulog::type::log, \
          __PRETTY_FUNCTION__,__FILE__,__LINE__,t})))

#define LBLOCK_BEGIN(l) ((l).dispatch_log_message(\
          mulog::log_header({mulog::info, mulog::type::block_begin, \
          __PRETTY_FUNCTION__,__FILE__,__LINE__,"__block_begin"})))
#define LBLOCK_END(L) ((l).dispatch_log_message(\
          mulog::log_header({mulog::info, mulog::type::block_end, \
          __PRETTY_FUNCTION__,__FILE__,__LINE__,"__block_end"})))
#define LHEADER(l) ((l).dispatch_log_message(\
          mulog::log_header({mulog::info, mulog::type::header, \
          __PRETTY_FUNCTION__,__FILE__,__LINE__,"__header"})))
#define LINFO(l) ((l).dispatch_log_message(mulog::log_header(\
          {mulog::info, mulog::type::log, \
          __PRETTY_FUNCTION__,__FILE__,__LINE__,""})))
#define LLOG(l,s) ((l).dispatch_log_message(mulog::log_header(\
          {mulog::s, mulog::type::log, \
          __PRETTY_FUNCTION__,__FILE__,__LINE__,""})))
#define LLOG_TAG(l,s,t) ((l).dispatch_log_message(mulog::log_header(\
          {mulog::s, mulog::type::log, \
          __PRETTY_FUNCTION__,__FILE__,__LINE__,t})))

// TODO:
// * Add headers and blocks
// * Add additional transformer types (vector<double>, vector<int>, matrices)
// * Add dispatcher transformers (Eigen -> vector<vector<>> and so on)
// * Add formatting types
// * HTML transformer + d3.js visuals
// * More filters
// * TCLAP compability for filters/min_severity and file output
// * Thread safety with C++11 thread library

namespace mulog
{

  enum log_severity
  {
    verbose3 = 0,
    verbose2,
    verbose,
    debug,
    info,
    warn,
    error,
    fatal
  };

  namespace type
  {
    enum log_type
    {
      log = 0,
      header,
      block_begin,
      block_end
    };
  }

  struct log_header
  {
    log_severity severity;
    type::log_type type;
    std::string function;
    std::string file;
    unsigned int loc;
    std::string tag;
  };

  namespace prefix
  {
    enum text_prefix
    {
      none = -1,
      severity = 0,
      timestamp,
      extended
    };
  }

  typedef std::function<bool (const log_header&)> header_filter;

  namespace filters
  {
    struct regex
    {
      regex(std::string && r) : filter(r) {};
      regex(const std::string & r) : filter(r) {};
    protected:
      std::regex filter;
    };

    struct tag : regex
    {
      tag(std::string && r) : regex(r) {};
      tag(const std::string & r) : regex(r) {};
      bool operator() (const log_header& h)
      {
        return std::regex_match(h.tag, filter);
      }
    };

    struct file : regex
    {
      file(std::string && r) : regex(r) {};
      file(const std::string & r) : regex(r) {};
      bool operator() (const log_header& h)
      {
        return std::regex_match(h.file, filter);
      }
    };

    struct function : regex
    {
      function(std::string && r) : regex(r) {};
      function(const std::string & r) : regex(r) {};
      bool operator() (const log_header& h)
      {
        return std::regex_match(h.function, filter);
      }
    };

    template<typename T, typename ... P>
    struct exclude
    {
      exclude(P ... params) : filter(params...) {};
      bool operator() (const log_header& h)
      {
        return !filter(h);
      }
    private:
      T filter;
    };
  }

  struct console_device
  {
    void write(const std::string& s) { std::cout << s; }
    std::ostream& dev_stream() { return std::cout; }
    void flush() {};
  };

  struct file_device
  {
    file_device(const std::string& filename) { file.open(filename); }
    ~file_device() { file.close();}

    void write(const std::string& s) { file << s; }
    std::ostream& dev_stream() { return file; }
    void flush() { file.flush(); };
  private:
    std::ofstream file;
  };

  struct transformer
  {
    virtual void begin_log(const log_header& h) = 0;
    virtual void end_log() = 0;
    virtual void log(const std::string& s) = 0;

    template<typename T>
    void log(const T& data)
    {
      std::stringstream s;
      s << data;
      log(s.str());
    }

  };

  typedef std::list<std::shared_ptr<transformer>> transformer_list;

  template<typename Device, typename Prefix, typename ... DeviceParams>
  struct default_transformer : transformer
  {
    default_transformer(Prefix p, DeviceParams&... params) : dev(params...), prefix(p) {};

    void begin_log(const log_header& h)
    {
      t = h.type;

      if(t == type::header)
      {
        dev.dev_stream() << std::endl;
      }
      else if(t == type::block_begin)
      {
        dev.dev_stream() << std::endl;
        dev.dev_stream() << "=======================================================" << std::endl;
      }
      else if(t == type::block_end)
      {
        dev.dev_stream() << "=======================================================" << std::endl << std::endl;
      }
      else if(t == type::log)
      {
        if(prefix == prefix::none)
          return;

        dev.dev_stream() << "[";
        switch(h.severity)
        {
        case verbose3:
          dev.dev_stream() << "VERB3";
          break;
        case verbose2:
          dev.dev_stream() << "VERB2";
          break;
        case verbose:
          dev.dev_stream() << "VERB";
          break;
        case debug:
          dev.dev_stream() << "DEBUG";
          break;
        case info:
          dev.dev_stream() << "INFO";
          break;
        case warn:
          dev.dev_stream() << "WARN";
          break;
        case error:
          dev.dev_stream() << "ERR";
          break;
        case fatal:
          dev.dev_stream() << "FATAL";
          break;
        }

        //typedef std::chrono::high_resolution_clock clock;
        //typedef std::chrono::milliseconds milliseconds;

        if(prefix == prefix::timestamp || prefix == prefix::extended)
        {
          time_t rawtime;
          struct tm * timeinfo;
          char buffer [80];
          time ( &rawtime );
          timeinfo = localtime ( &rawtime );
          strftime (buffer,80,"%Y-%m-%d %X",timeinfo);
          dev.dev_stream() << " " << buffer;
        }

        if(prefix == prefix::extended)
        {
          dev.dev_stream() << " " << h.file << ":" << h.loc << " - " << h.function;
          if(h.tag.length() > 0)
            dev.dev_stream() << " (" << h.tag << ")";
        }

        dev.dev_stream() << "] ";
      }
    }

    void log(const std::string& s) { dev.write(s); }
    void end_log()
    {
      if(t == type::log)
      {
        dev.dev_stream() << std::endl;
      }
      else if(t == type::header)
      {
        dev.dev_stream() << std::endl;
        dev.dev_stream() << "-------------------------------------------------------" << std::endl;
      }
      else if(t == type::block_begin)
      {
        dev.dev_stream() << std::endl;
        dev.dev_stream() << "=======================================================" << std::endl;
      }
      dev.flush();
    }

    Device dev;
    prefix::text_prefix prefix;
    type::log_type t;
  };

  struct dispatcher
  {
    dispatcher() : active(false), tl(nullptr) {};
    dispatcher(const log_header& header,
               const transformer_list& transformers) : active(true),
                                                       tl(&transformers)
    {
      for(auto t : (*tl))
        t->begin_log(header);
    };
    ~dispatcher()
    {
      if(active && tl != nullptr)
        for(auto t : (*tl))
          t->end_log();
    };

    bool active;
    const transformer_list* tl;
  };

  template<typename T>
  dispatcher& operator << (dispatcher& d, const T& data)
  {
    if(d.active && d.tl != nullptr)
      for(auto t : (* d.tl))
        t->log(data);
    return d;
  }

  template<typename T>
  dispatcher& operator << (dispatcher& d, const T&& data)
  {
    if(d.active && d.tl != nullptr)
      for(auto t : (* d.tl))
        t->log(data);
    return d;
  }

  typedef std::shared_ptr<dispatcher> dispatcher_handle;

  class logger
  {
  public:
    logger() : min_severity(info)  {};
    ~logger() {};

    void set_min_severity(log_severity s) {  min_severity = s; };
    void add_filter(header_filter&& f) {  header_filters.push_back(f); };

    template<template<typename, typename...> class T, typename D, typename ... TP>
    void add_transformer(TP ... params)
    {
      transformers.push_back(std::shared_ptr<transformer>(new T<D, TP...>(params...)));
    };

    dispatcher_handle dispatch_log_message(log_header&& header)
    {
      bool dispatch = true;

      for(auto hf : header_filters)
        if(!hf(header))
          dispatch = false;

      if(dispatch && header.severity >= min_severity)
        return dispatcher_handle(new dispatcher(header, transformers));

      return dispatcher_handle(new dispatcher);
    }

    dispatcher_handle dispatch_log_message(const log_header& header)
    {
      bool dispatch = true;

      for(auto hf : header_filters)
        if(!hf(header))
          dispatch = false;

      if(dispatch && header.severity >= min_severity)
        return dispatcher_handle(new dispatcher(header, transformers));

      return dispatcher_handle(new dispatcher);
    }

  private:
    log_severity min_severity;
    std::list<header_filter> header_filters;
    transformer_list transformers;
  };

  namespace core
  {
    inline logger& get()
    {
      static logger global_logger;
      return global_logger;
    };
  }
}

#endif /* _MULOG_H_ */
