#ifndef RAPID_PROFILE__HPP_
#define RAPID_PROFILE__HPP_

#include <list>
#include <vector>
#include <chrono>
#include <iostream>
#include <cstring>
#include <csignal>
#include <mutex>

#define NNN 32
#define CHUNK_SIZE 1048576

namespace RapidProfile
{

namespace type
{
typedef unsigned int id;
typedef float time;
// typedef float duration;
typedef std::chrono::steady_clock clock;
typedef clock::time_point time_point;
typedef clock::duration duration;

} // namespace type

struct interval
{
    interval()
    {
    }
    interval(const type::id id) : id(id), start(type::clock::now())
    {
    }
    type::time duration() const
    {
        return std::chrono::duration<type::time>(stop - start).count();
    }
    type::id id;
    type::time_point start;
    type::time_point stop;

    template <size_t N>
    struct info
    {
        char name[N];
        char file[N];
        int line;
    };
};

template <class T>
class chunker
{
  public:
    chunker(size_t chunk_size) : chunk_size_(chunk_size), chunk_(NULL)
    {
        chunk_ = new std::vector<T>();
        chunk_->reserve(chunk_size_);
        chunks_.push_back(chunk_);

        rechunk();
    }

    ~chunker()
    {
        for (typename std::list<std::vector<T> *>::iterator it = chunks_.begin(); it != chunks_.end(); it++)
            delete *it;
        chunks_.clear();
    }

    T &push_back(T const &item = T())
    {
        if (chunk_->size() == chunk_size_)
            rechunk();

        chunk_->push_back(item);
        return chunk_->back();
    }

    std::list<std::vector<T> *> &chunks()
    {
        return chunks_;
    }

  protected:
    void rechunk()
    {
        chunk_ = chunks_.back();

        std::vector<T> *next = new std::vector<T>();
        next->reserve(chunk_size_);
        chunks_.push_back(next);
    }

  private:
    size_t chunk_size_;
    std::vector<T> *chunk_;
    std::list<std::vector<T> *> chunks_;
};

template <size_t N>
class api
{
  public:
    static void init()
    {
        interval_info();
        interval_info().reserve(1000);
        since_start();
        std::atexit(api::exit);
        ::signal(SIGINT, api::signal);
    }

    static void exit()
    {
        log();
    }

    static void signal(int signum) {
        ::exit(signum);     
    }

    static interval &get_interval()
    {
        std::lock_guard<std::mutex> guard(mutex());
        return intervals().push_back();
    }

    static type::id get_id(const char *name, const char *file, int line)
    {
        static type::id id = 0;
        std::lock_guard<std::mutex> guard(mutex());
        interval::info<N> info_;
        interval_info().push_back(info_);
        interval::info<N> &info = interval_info().back();
        strncpy(info.name, name, N);
        info.name[N - 1] = '\0';
        strncpy(info.file, file, N);
        info.file[N - 1] = '\0';
        info.line = line;
        return id++;
    }

    static type::time relative(type::time_point const &start, type::time_point const &stop = type::clock::now())
    {
        return std::chrono::duration<type::time>(stop - start).count();
    }

    static type::time since_start(type::time_point const &time = type::clock::now())
    {
        static type::time_point start_time = type::clock::now();
        return std::chrono::duration<type::time>(time - start_time).count();
    }

    static void log()
    {
        std::list<std::vector<interval> *> &chunks = intervals().chunks();
        for (std::list<std::vector<interval> *>::iterator it = chunks.begin(); it != chunks.end(); it++)
        {
            for (std::vector<interval>::iterator vit = (*it)->begin(); vit != (*it)->end(); vit++)
            {
                std::cout << interval_info()[vit->id].name << " @ (" << interval_info()[vit->id].file << ":" << interval_info()[vit->id].line << ") " << relative(vit->start, vit->stop) * 1e6 << " us " << std::endl;
            }
        }
    }

  private:
    static std::mutex & mutex()
    {
        static std::mutex mutex;
        return mutex;
    }

    static std::vector<interval::info<N>> &interval_info()
    {
        static std::vector<interval::info<N>> interval_info;
        return interval_info;
    }

    static chunker<interval> &intervals()
    {
        static chunker<interval> *intervals = new chunker<interval>(CHUNK_SIZE);
        return *intervals;
    }
};

#define RAPID_PROFILE_INIT() \
    RapidProfile::api<NNN>::init()

#define INTERVAL_ID(NAME) _rapid_profile_interval_##NAME##_id
#define INTERVAL_INST(NAME) _rapid_profile_interval_##NAME

#define INTERVAL(NAME)                                                                                           \
    static RapidProfile::type::id INTERVAL_ID(NAME) = RapidProfile::api<NNN>::get_id(#NAME, __FILE__, __LINE__); \
    RapidProfile::interval &INTERVAL_INST(NAME) = RapidProfile::api<NNN>::get_interval();                        \
    INTERVAL_INST(NAME).id = INTERVAL_ID(NAME);                                                                  \
    INTERVAL_INST(NAME).start = RapidProfile::type::clock::now();
    // std::cout << "start " << #NAME << std::endl;

#define INTERVAL_END(NAME) \
    INTERVAL_INST(NAME).stop = RapidProfile::type::clock::now();
    // std::cout << "stop " << #NAME << std::endl;

#define INTERVAL_START(NAME) \
    INTERVAL_INST(NAME).start = RapidProfile::type::clock::now();

}; // namespace RapidProfile

#endif
