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
    std::string op;
    std::string pattern;

    try {
        namespace po = boost::program_options;

        po::options_description desc(
            "keydb_tool\n"
            "Usage: keydb_tool [OPTION]...\n\n"
            "Options");
        desc.add_options()
            ("conf,c", po::value<std::string>(), "Set the configuration file.")
            ("help", "Print this help")
            ("op", po::value<std::string>(), "scan|delete|get")
            ("pattern", po::value<std::string>(), "glob-style pattern (for scan|delete)")
            ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return EXIT_SUCCESS;
        }

        if (!vm.count("op")) {
            std::cout << "Please enter the op.\n\n"
                << desc << std::endl;
            return EXIT_FAILURE;
        } else {
            op = vm["op"].as<std::string>();
            if ((op != "scan") && (op != "delete") && (op != "get")) {
                std::cout << "op can only be scan|delete|get.\n\n"
                    << desc << std::endl;
                return EXIT_FAILURE;
            }
        }

        if (vm.count("pattern")) {
            pattern = vm["pattern"].as<std::string>();
        } else {
            if (op == "scan" || op == "delete") {
                std::cout << "Please enter the pattern.\n\n"
                    << desc << std::endl;
                return EXIT_FAILURE;
            }
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
                    "keydb_tool.log",
                    LogHard::FileAppender::OVERWRITE));
    const LogHard::Logger logger(logBackend);

    logger.info() << "Input parameters:";

    {
        logger.info() << "op: " << op;
        if (op == "scan" || op == "delete") {
            logger.info() << "pattern: " << pattern;
        }
    }
    
    try {
        // upload
        {
            sm::SystemControllerGlobals systemControllerGlobals;
            sm::SystemController c(logger, *config);

            // Initialize the argument map and set the argument
            sm::SystemController::ValueMap arguments;
            arguments["op"] =
                    std::make_shared<sm::SystemController::Value>(
                        "",
                        "string",
                        newGlobalBuffer((const char *)op.c_str(), sizeof(char) * op.length()),
                        sizeof(char) * op.length());
            arguments["pattern"] =
                    std::make_shared<sm::SystemController::Value>(
                        "",
                        "string",
                        newGlobalBuffer((const char *)pattern.c_str(), sizeof(char) * pattern.length()),
                        sizeof(char) * pattern.length());

            // Run code
            logger.info() << "Executing SecreC program";
            sm::SystemController::ValueMap results = c.runCode("keydb_tool.sb", arguments);
        }

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
