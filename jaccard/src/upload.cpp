#include <boost/program_options.hpp>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <LogHard/Backend.h>
#include <LogHard/FileAppender.h>
#include <LogHard/Logger.h>
#include <LogHard/StdAppender.h>
#include <memory>
#include <sharemind/controller/SystemController.h>
#include <sharemind/controller/SystemControllerConfiguration.h>
#include <sharemind/controller/SystemControllerGlobals.h>
#include <sharemind/DebugOnly.h>
#include <sharemind/GlobalDeleter.h>
// #include <sharemind/MakeUnique.h>
#include <sstream>
#include <string>

namespace sm = sharemind;

inline std::shared_ptr<void> newGlobalBuffer(std::size_t const size) {
    auto * const b = size ? ::operator new(size) : nullptr;
    try {
        return std::shared_ptr<void>(b, sm::GlobalDeleter());
    } catch (...) {
        ::operator delete(b);
        throw;
    }
}

inline std::shared_ptr<void> newGlobalBuffer(void const * const data,
                                             std::size_t const size)
{
    auto r(newGlobalBuffer(size));
    std::memcpy(r.get(), data, size);
    return r;
}

inline std::string int_to_hex(std::uint64_t i)
{
    std::stringstream stream;
    stream << "0x" << std::hex << i;
    return stream.str();
}

int main(int argc, char ** argv) {
    std::unique_ptr<sm::SystemControllerConfiguration> config;
    auto tokens = std::make_shared<std::vector<std::uint64_t>>();
    std::vector<std::string> tokens_str;
    std::string key;

    try {
        namespace po = boost::program_options;

        po::options_description desc(
            "upload\n"
            "Usage: upload [OPTION]...\n\n"
            "Options");
        desc.add_options()
            ("conf,c", po::value<std::string>(), "Set the configuration file.")
            ("help", "Print this help")
            ("key,k", po::value<std::string>(), "key in Redis")
            ("tokens,t", po::value<std::vector<std::string>>()->multitoken(), "tokens,t")
            ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return EXIT_SUCCESS;
        }

        if (!vm.count("tokens")) {
            std::cout << "Please enter the tokens.\n\n"
                << desc << std::endl;
            return EXIT_FAILURE;
        } else {
            tokens_str = vm["tokens"].as< std::vector<std::string> >();
            for (const auto s : tokens_str) {
                tokens->push_back(std::stol(s, nullptr, 0));
            }
        }
        if (!vm.count("key")) {
            std::cout << "Please enter the key.\n\n"
                << desc << std::endl;
            return EXIT_FAILURE;
        } else {
            key = vm["key"].as<std::string>();
        }

        if (vm.count("conf")) {
            config = std::make_unique<sm::SystemControllerConfiguration>(
                        vm["conf"].as<std::string>());
        } else {
            config = std::make_unique<sm::SystemControllerConfiguration>();
        }
    } catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    auto logBackend(std::make_shared<LogHard::Backend>());
    logBackend->addAppender(std::make_shared<LogHard::StdAppender>());
    logBackend->addAppender(
                std::make_shared<LogHard::FileAppender>(
                    "upload.log",
                    LogHard::FileAppender::OVERWRITE));
    const LogHard::Logger logger(logBackend);

    logger.info() << "Starting uploading...";
    logger.info() << "Input parameters:";

    {
        logger.info() << "key: " << key;
    }

    {
        std::ostringstream oss;
        oss << "tokens: [ ";
        for (const auto val : *tokens) {
            oss << int_to_hex(val) << ' ';
        }
        oss << ']';
        logger.info() << oss.str();
    }

    try {
        sm::SystemControllerGlobals systemControllerGlobals;
        sm::SystemController c(logger, *config);

        // Initialize the argument map and set the argument
        sm::SystemController::ValueMap arguments;
        arguments["key"] =
                std::make_shared<sm::SystemController::Value>(
                    "",
                    "string",
                    newGlobalBuffer((const char *)key.c_str(), sizeof(char) * key.length()),
                    sizeof(char) * key.length());
        arguments["tokens"] =
                std::make_shared<sm::SystemController::Value>(
                    "pd_shared3p",
                    "uint64",
                    std::shared_ptr<std::uint64_t>(tokens, tokens->data()),
                    sizeof(std::uint64_t) * tokens->size());

        // Run code
        logger.info() << "Sending secret shared arguments and running SecreC bytecode on the servers";
        sm::SystemController::ValueMap results = c.runCode("upload.sb", arguments);

    } catch (const sm::SystemController::WorkerException & e) {
        logger.fatal() << "Multiple exceptions caught:";
        for (size_t i = 0u; i < e.numWorkers(); i++) {
            if (std::exception_ptr ep = e.nested_ptrs()[i]) {
                logger.fatal() << "  Exception from server " << i << ':';
                try {
                    std::rethrow_exception(std::move(ep));
                } catch (...) {
                    logger.printCurrentException<LogHard::Priority::Fatal>(
                            LogHard::Logger::StandardExceptionFormatter(4u));
                }
            }
        }
    } catch (const std::exception & e) {
        logger.error() << "Caught exception: " << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
