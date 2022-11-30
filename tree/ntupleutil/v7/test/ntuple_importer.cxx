#include "gtest/gtest.h"

#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleImporter.hxx>

#include <TFile.h>
#include <TTree.h>

#include <cstdio>
#include <string>

using ROOT::Experimental::RNTupleImporter;
using ROOT::Experimental::RNTupleReader;

/**
 * An RAII wrapper around an open temporary file on disk. It cleans up the guarded file when the wrapper object
 * goes out of scope.
 */
class FileRaii {
private:
   std::string fPath;

public:
   explicit FileRaii(const std::string &path) : fPath(path) {}
   FileRaii(const FileRaii &) = delete;
   FileRaii &operator=(const FileRaii &) = delete;
   ~FileRaii() { std::remove(fPath.c_str()); }
   std::string GetPath() const { return fPath; }
};

TEST(RNTupleImporter, Empty)
{
   FileRaii fileGuard("test_ntuple_importer_empty.root");
   {
      std::unique_ptr<TFile> file(TFile::Open(fileGuard.GetPath().c_str(), "RECREATE"));
      auto tree = std::make_unique<TTree>("tree", "");
      tree->Write();
   }

   auto importer = RNTupleImporter::Create(fileGuard.GetPath(), "tree", fileGuard.GetPath()).Unwrap();
   importer->SetIsQuiet(true);
   EXPECT_THROW(importer->Import(), ROOT::Experimental::RException);
   importer->SetNTupleName("ntuple");
   importer->Import();
   auto reader = RNTupleReader::Open("ntuple", fileGuard.GetPath());
   EXPECT_EQ(0U, reader->GetNEntries());
   EXPECT_THROW(importer->Import(), ROOT::Experimental::RException);
}

TEST(RNTupleImporter, Simple)
{
   FileRaii fileGuard("test_ntuple_importer_simple.root");
   {
      std::unique_ptr<TFile> file(TFile::Open(fileGuard.GetPath().c_str(), "RECREATE"));
      auto tree = std::make_unique<TTree>("tree", "");
      bool myBool = true;
      Char_t myInt8 = -8;
      UChar_t myUInt8 = 8;
      Short_t myInt16 = -16;
      UShort_t myUInt16 = 16;
      Int_t myInt32 = -32;
      UInt_t myUInt32 = 32;
      Long64_t myInt64 = -64;
      ULong64_t myUInt64 = 64;
      Float_t myFloat = 32.0;
      Double_t myDouble = 64.0;
      // TODO(jblomer): Float16_t, Double32_t
      tree->Branch("myBool", &myBool);
      tree->Branch("myInt8", &myInt8);
      tree->Branch("myUInt8", &myUInt8);
      tree->Branch("myInt16", &myInt16);
      tree->Branch("myUInt16", &myUInt16);
      tree->Branch("myInt32", &myInt32);
      tree->Branch("myUInt32", &myUInt32);
      tree->Branch("myInt64", &myInt64);
      tree->Branch("myUInt64", &myUInt64);
      tree->Branch("myFloat", &myFloat);
      tree->Branch("myDouble", &myDouble);
      tree->Fill();
      tree->Write();
   }

   auto importer = RNTupleImporter::Create(fileGuard.GetPath(), "tree", fileGuard.GetPath()).Unwrap();
   importer->SetIsQuiet(true);
   importer->SetNTupleName("ntuple");
   importer->Import();
   auto reader = RNTupleReader::Open("ntuple", fileGuard.GetPath());
   EXPECT_EQ(1U, reader->GetNEntries());
   reader->LoadEntry(0);
   EXPECT_TRUE(*reader->GetModel()->Get<bool>("myBool"));
   EXPECT_EQ(-8, *reader->GetModel()->Get<char>("myInt8"));
   EXPECT_EQ(8U, *reader->GetModel()->Get<std::uint8_t>("myUInt8"));
   EXPECT_EQ(-16, *reader->GetModel()->Get<std::int16_t>("myInt16"));
   EXPECT_EQ(16U, *reader->GetModel()->Get<std::uint16_t>("myUInt16"));
   EXPECT_EQ(-32, *reader->GetModel()->Get<std::int32_t>("myInt32"));
   EXPECT_EQ(32U, *reader->GetModel()->Get<std::uint32_t>("myUInt32"));
   EXPECT_EQ(-64, *reader->GetModel()->Get<std::int64_t>("myInt64"));
   EXPECT_EQ(64U, *reader->GetModel()->Get<std::uint64_t>("myUInt64"));
   EXPECT_FLOAT_EQ(32.0, *reader->GetModel()->Get<float>("myFloat"));
   EXPECT_FLOAT_EQ(64.0, *reader->GetModel()->Get<double>("myDouble"));
}

TEST(RNTupleImporter, CString)
{
   FileRaii fileGuard("test_ntuple_importer_cstring.root");
   {
      std::unique_ptr<TFile> file(TFile::Open(fileGuard.GetPath().c_str(), "RECREATE"));
      auto tree = std::make_unique<TTree>("tree", "");
      const char *myString = "R";
      tree->Branch("myString", const_cast<char *>(myString), "myString/C");
      tree->Fill();
      myString = "";
      tree->SetBranchAddress("myString", const_cast<char *>(myString));
      tree->Fill();
      myString = "ROOT RNTuple";
      tree->SetBranchAddress("myString", const_cast<char *>(myString));
      tree->Fill();
      tree->Write();
   }

   auto importer = RNTupleImporter::Create(fileGuard.GetPath(), "tree", fileGuard.GetPath()).Unwrap();
   importer->SetIsQuiet(true);
   importer->SetNTupleName("ntuple");
   importer->Import();
   auto reader = RNTupleReader::Open("ntuple", fileGuard.GetPath());
   EXPECT_EQ(3U, reader->GetNEntries());
   reader->LoadEntry(0);
   EXPECT_EQ(std::string("R"), *reader->GetModel()->Get<std::string>("myString"));
   reader->LoadEntry(1);
   EXPECT_EQ(std::string(""), *reader->GetModel()->Get<std::string>("myString"));
   reader->LoadEntry(2);
   EXPECT_EQ(std::string("ROOT RNTuple"), *reader->GetModel()->Get<std::string>("myString"));
}

TEST(RNTupleImporter, Leaflist)
{
   FileRaii fileGuard("test_ntuple_importer_leaflist.root");
   {
      std::unique_ptr<TFile> file(TFile::Open(fileGuard.GetPath().c_str(), "RECREATE"));
      auto tree = std::make_unique<TTree>("tree", "");
      struct {
         Int_t a = 1;
         Int_t b = 2;
      } leafList;
      tree->Branch("branch", &leafList, "a/I:b/I");
      tree->Fill();
      tree->Write();
   }

   auto importer = RNTupleImporter::Create(fileGuard.GetPath(), "tree", fileGuard.GetPath()).Unwrap();
   importer->SetIsQuiet(true);
   importer->SetNTupleName("ntuple");
   importer->Import();

   auto reader = RNTupleReader::Open("ntuple", fileGuard.GetPath());
   EXPECT_EQ(1U, reader->GetNEntries());
   // Field "branch" is an anonymous record, we cannot go through the default model here
   auto viewA = reader->GetView<std::int32_t>("branch.a");
   auto viewB = reader->GetView<std::int32_t>("branch.b");
   EXPECT_EQ(1, viewA(0));
   EXPECT_EQ(2, viewB(0));
}

TEST(RNTupleImporter, FixedSizeArray)
{
   FileRaii fileGuard("test_ntuple_importer_fixed_size_array.root");
   {
      std::unique_ptr<TFile> file(TFile::Open(fileGuard.GetPath().c_str(), "RECREATE"));
      auto tree = std::make_unique<TTree>("tree", "");
      Int_t a[1] = {42};
      Int_t b[2] = {1, 2};
      char c[4] = {'R', 'O', 'O', 'T'};
      tree->Branch("a", a, "a[1]/I");
      tree->Branch("b", b, "b[2]/I");
      tree->Branch("c", c, "c[4]/C");
      tree->Fill();
      tree->Write();
   }

   auto importer = RNTupleImporter::Create(fileGuard.GetPath(), "tree", fileGuard.GetPath()).Unwrap();
   importer->SetIsQuiet(true);
   importer->SetNTupleName("ntuple");
   importer->Import();

   auto reader = RNTupleReader::Open("ntuple", fileGuard.GetPath());
   EXPECT_EQ(1U, reader->GetNEntries());
   auto viewA = reader->GetView<std::int32_t>("a");
   auto viewB = reader->GetView<std::array<std::int32_t, 2>>("b");
   auto viewC = reader->GetView<std::array<char, 4>>("c");
   EXPECT_EQ(42, viewA(0));
   EXPECT_EQ(2U, viewB(0).size());
   EXPECT_EQ(1, viewB(0)[0]);
   EXPECT_EQ(2, viewB(0)[1]);
   EXPECT_EQ(4U, viewC(0).size());
   EXPECT_EQ('R', viewC(0)[0]);
   EXPECT_EQ('O', viewC(0)[1]);
   EXPECT_EQ('O', viewC(0)[2]);
   EXPECT_EQ('T', viewC(0)[3]);
}
