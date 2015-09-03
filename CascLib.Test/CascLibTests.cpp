#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <fstream>
#include <memory>
#include <thread>
#include <vector>

#include "../CascLib/Casc/Common.hpp"

using namespace Casc;
 
namespace CascLibTest
{
	TEST_CLASS(CascLibTests)
	{
	public:

        TEST_METHOD(LoadContainer)
        {
            auto container = std::make_unique<CascContainer>(
                R"(I:\Diablo III\)",
                "Data",
                std::vector<std::shared_ptr<CascBlteHandler>> {
                    std::make_shared<ZlibHandler>()
            });
        }

		TEST_METHOD(GetRootFile)
		{
            auto container = std::make_unique<CascContainer>(
                R"(I:\Diablo III\)",
                "Data",
                std::vector<std::shared_ptr<CascBlteHandler>> {
                    std::make_shared<ZlibHandler>()
            });
			auto root = container->openFileByHash(container->buildConfig()["root"].front());
            
            root->seekg(0, std::ios_base::end);
            auto size = root->tellg();

            std::fstream fs;
            char* arr;

            char magic[4];
            int count;

            root->seekg(0, std::ios_base::beg);
            root->read(magic, 4);
            root->read((char*)&count, 4);

            /*for (int i = 0; i < count; ++i)
            {
                std::array<uint8_t, 16> hash;
                std::string name;

                root->read(reinterpret_cast<char*>(&hash[0]), 16);
                std::getline(*root.get(), name, '\0');

                Hex hex(hash);

                try
                {
                    auto dir = container->openFileByHash(hex.string());
                    dir->seekg(0, std::ios_base::end);
                    auto dirSize = dir->tellg();

                    dir->seekg(0, std::ios_base::beg);
                    arr = new char[(size_t)dirSize];

                    dir->read(arr, dirSize);

                    fs.open(name, std::ios_base::out | std::ios_base::binary);

                    fs.write(arr, dirSize);
                    fs.close();

                    delete[] arr;
                }
                catch (...)
                {
                    continue;
                }
            }*/

            root->seekg(0, std::ios_base::beg);
            arr = new char[(size_t)size];

            root->read(arr, size);

            fs.open("root.d3.out", std::ios_base::out | std::ios_base::binary);

            fs.write(arr, size);
            fs.close();

            delete[] arr;
		}

		TEST_METHOD(GetEncodingFile)
		{
            auto container = std::make_unique<CascContainer>(
                R"(I:\Diablo III\)",
                "Data",
                std::vector<std::shared_ptr<CascBlteHandler>> {
                    std::make_shared<ZlibHandler>()
            });

			auto key = container->encoding().find(container->buildConfig()["root"].front());
            container->openFileByKey(key.at(0).string());

            auto enc = container->openFileByKey(container->buildConfig()["encoding"].back());

            enc->seekg(0, std::ios_base::end);
            auto size = enc->tellg();

            enc->seekg(0, std::ios_base::beg);
            auto arr = new char[(size_t)size];

            enc->read(arr, size);

            std::fstream fs;
            fs.open("enc.d3.out", std::ios_base::out | std::ios_base::binary);

            fs.write(arr, size);
            fs.close();

            delete[] arr;
		}

        TEST_METHOD(GetUnknownFile)
        {
            auto container = std::make_unique<CascContainer>(
                R"(I:\World of Warcraft\)",
                "Data",
                std::vector<std::shared_ptr<CascBlteHandler>> {
                std::make_shared<ZlibHandler>()
            });
            /*CascEncoding enc(
            container->openFileByKey(container->buildConfig()["encoding"].back()));

            auto key = enc.find(container->buildConfig()["root"].front());*/

            auto enc = container->openFileByKey("0000078f0af7715be04aef1aaed38b90");

            enc->seekg(0, std::ios_base::end);
            auto size = enc->tellg();

            enc->seekg(0, std::ios_base::beg);
            auto arr = new char[(size_t)size];

            enc->read(arr, size);

            std::fstream fs;
            fs.open("0000078f0af7715be04aef1aaed38b90", std::ios_base::out | std::ios_base::binary);

            fs.write(arr, size);
            fs.close();

            delete[] arr;
        }

		TEST_METHOD(ReadConfiguration)
		{
			CascConfiguration configuration(R"(I:\Diablo III\Data\config\0d\a0\0da08d69484c74c91e50aab485f5b4ba)");
		}

		TEST_METHOD(ReadBuildInfo)
		{
			CascBuildInfo buildInfo(R"(I:\Diablo III\.build.info)"); 
		}

		TEST_METHOD(ReadShmem)
		{
            CascShmem shmem(R"(shmem)", R"(I:\Diablo III\)");
		}

        TEST_METHOD(WriteFile)
        {
            auto container = std::make_unique<CascContainer>(
                R"(I:\Diablo III\)",
                "Data",
                std::vector<std::shared_ptr<CascBlteHandler>> {
                    std::make_shared<ZlibHandler>()
            });

            std::fstream fs;
            fs.open(R"(C:\Users\Gunnar\Source\Repos\CascLib\Fast\casc.exe)", std::ios_base::in | std::ios_base::binary);

            if (fs.fail())
            {
                throw std::runtime_error("Failed to open file.");
            }

            fs.seekg(0, std::ios_base::end);
            auto size = fs.tellg();

            fs.seekg(0, std::ios_base::beg);

            /*container->write(fs, CascLayoutDescriptor({
                CascChunkDescriptor(CompressionMode::None, 0, (size_t)size) }));*/

            auto enc = container->encoding().insert("", "", 0);
        }

        TEST_METHOD(GetBucket)
        {
            std::vector<uint8_t> vec{ 0x41, 0xEE, 0x19, 0x86, 0xAC, 0xC5, 0x33, 0xCC, 0x00 };
            std::array<uint8_t, 9> arr{ 0x41, 0xEE, 0x19, 0x86, 0xAC, 0xC5, 0x33, 0xCC, 0x00 };

            auto a = CascIndex::bucket(arr.begin(), arr.end());
            auto b = CascIndex::bucket(vec.begin(), vec.end());

            Assert::AreEqual(a, b);
        }

	};
}