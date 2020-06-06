#include "pch.h"
#include <filesystem>
#include <fstream>

#include <lib/JsonHelpers.h>
#include "util.h"

#include <CppUnitTestLogger.h>

using namespace JSONHelpers;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace FancyZonesUnitTests
{
    void compareJsonObjects(const json::JsonObject& expected, const json::JsonObject& actual, bool recursive = true)
    {
        auto iter = expected.First();
        while (iter.HasCurrent())
        {
            const auto key = iter.Current().Key();
            Assert::IsTrue(actual.HasKey(key), key.c_str());

            const std::wstring expectedStringified = iter.Current().Value().Stringify().c_str();
            const std::wstring actualStringified = actual.GetNamedValue(key).Stringify().c_str();

            if (recursive)
            {
                json::JsonObject expectedJson;
                if (json::JsonObject::TryParse(expectedStringified, expectedJson))
                {
                    json::JsonObject actualJson;
                    if (json::JsonObject::TryParse(actualStringified, actualJson))
                    {
                        compareJsonObjects(expectedJson, actualJson, true);
                    }
                    else
                    {
                        Assert::IsTrue(false, key.c_str());
                    }
                }
                else
                {
                    Assert::AreEqual(expectedStringified, actualStringified, key.c_str());
                }
            }
            else
            {
                Assert::AreEqual(expectedStringified, actualStringified, key.c_str());
            }

            iter.MoveNext();
        }
    }

    TEST_CLASS (IdValidationUnitTest)
    {
        TEST_METHOD (GuidValid)
        {
            const auto guidStr = Helpers::CreateGuidString();
            Assert::IsTrue(isValidGuid(guidStr));
        }

        TEST_METHOD (GuidInvalidForm)
        {
            const auto guidStr = L"33A2B101-06E0-437B-A61E-CDBECF502906";
            Assert::IsFalse(isValidGuid(guidStr));
        }

        TEST_METHOD (GuidInvalidSymbols)
        {
            const auto guidStr = L"{33A2B101-06E0-437B-A61E-CDBECF50290*}";
            Assert::IsFalse(isValidGuid(guidStr));
        }

        TEST_METHOD (GuidInvalid)
        {
            const auto guidStr = L"guid";
            Assert::IsFalse(isValidGuid(guidStr));
        }

        TEST_METHOD (DeviceId)
        {
            const auto deviceId = L"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1539}";
            Assert::IsTrue(isValidDeviceId(deviceId));
        }

        TEST_METHOD (DeviceIdWithoutHashInName)
        {
            const auto deviceId = L"LOCALDISPLAY_5120_1440_{00000000-0000-0000-0000-000000000000}";
            Assert::IsTrue(isValidDeviceId(deviceId));
        }

        TEST_METHOD (DeviceIdWithoutHashInNameButWithUnderscores)
        {
            const auto deviceId = L"LOCAL_DISPLAY_5120_1440_{00000000-0000-0000-0000-000000000000}";
            Assert::IsFalse(isValidDeviceId(deviceId));
        }

        TEST_METHOD (DeviceIdWithUnderscoresInName)
        {
            const auto deviceId = L"Default_Monitor#1&1f0c3c2f&0&UID256_5120_1440_{00000000-0000-0000-0000-000000000000}";
            Assert::IsTrue(isValidDeviceId(deviceId));
        }

        TEST_METHOD (DeviceIdInvalidFormat)
        {
            const auto deviceId = L"_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1539}";
            Assert::IsFalse(isValidDeviceId(deviceId));
        }

        TEST_METHOD (DeviceIdInvalidFormat2)
        {
            const auto deviceId = L"AOC2460#4&fe3a015&0&UID65793_19201200_{39B25DD2-130D-4B5D-8851-4791D66B1539}";
            Assert::IsFalse(isValidDeviceId(deviceId));
        }

        TEST_METHOD (DeviceIdInvalidDecimals)
        {
            const auto deviceId = L"AOC2460#4&fe3a015&0&UID65793_aaaa_1200_{39B25DD2-130D-4B5D-8851-4791D66B1539}";
            Assert::IsFalse(isValidDeviceId(deviceId));
        }

        TEST_METHOD (DeviceIdInvalidDecimals2)
        {
            const auto deviceId = L"AOC2460#4&fe3a015&0&UID65793_19a0_1200_{39B25DD2-130D-4B5D-8851-4791D66B1539}";
            Assert::IsFalse(isValidDeviceId(deviceId));
        }

        TEST_METHOD (DeviceIdInvalidDecimals3)
        {
            const auto deviceId = L"AOC2460#4&fe3a015&0&UID65793_1900_120000000000000_{39B25DD2-130D-4B5D-8851-4791D66B1539}";
            Assert::IsFalse(isValidDeviceId(deviceId));
        }

        TEST_METHOD (DeviceIdInvalidGuid)
        {
            const auto deviceId = L"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-4B5D-8851-4791D66B1539}";
            Assert::IsFalse(isValidDeviceId(deviceId));
        }
    };
    TEST_CLASS (ZoneSetLayoutTypeUnitTest)
    {
        TEST_METHOD (ZoneSetLayoutTypeToString)
        {
            std::map<int, std::wstring> expectedMap = {
                std::make_pair(-2, L"TypeToString_ERROR"),
                std::make_pair(-1, L"blank"),
                std::make_pair(0, L"focus"),
                std::make_pair(1, L"columns"),
                std::make_pair(2, L"rows"),
                std::make_pair(3, L"grid"),
                std::make_pair(4, L"priority-grid"),
                std::make_pair(5, L"custom"),
                std::make_pair(6, L"TypeToString_ERROR"),
            };

            for (const auto& expected : expectedMap)
            {
                auto actual = JSONHelpers::TypeToString(static_cast<ZoneSetLayoutType>(expected.first));
                Assert::AreEqual(expected.second, actual);
            }
        }

        TEST_METHOD (ZoneSetLayoutTypeFromString)
        {
            std::map<ZoneSetLayoutType, std::wstring> expectedMap = {
                std::make_pair(ZoneSetLayoutType::Focus, L"focus"),
                std::make_pair(ZoneSetLayoutType::Columns, L"columns"),
                std::make_pair(ZoneSetLayoutType::Rows, L"rows"),
                std::make_pair(ZoneSetLayoutType::Grid, L"grid"),
                std::make_pair(ZoneSetLayoutType::PriorityGrid, L"priority-grid"),
                std::make_pair(ZoneSetLayoutType::Custom, L"custom"),
            };

            for (const auto& expected : expectedMap)
            {
                auto actual = JSONHelpers::TypeFromString(expected.second);
                Assert::AreEqual(static_cast<int>(expected.first), static_cast<int>(actual));
            }
        }

        TEST_METHOD (ZoneSetLayoutTypeFromLayoutId)
        {
            std::map<ZoneSetLayoutType, int> expectedMap = {
                std::make_pair(ZoneSetLayoutType::Focus, 0xFFFF),
                std::make_pair(ZoneSetLayoutType::Columns, 0xFFFD),
                std::make_pair(ZoneSetLayoutType::Rows, 0xFFFE),
                std::make_pair(ZoneSetLayoutType::Grid, 0xFFFC),
                std::make_pair(ZoneSetLayoutType::PriorityGrid, 0xFFFB),
                std::make_pair(ZoneSetLayoutType::Blank, 0xFFFA),
                std::make_pair(ZoneSetLayoutType::Custom, 0),
                std::make_pair(ZoneSetLayoutType::Custom, 1),
                std::make_pair(ZoneSetLayoutType::Custom, -1),
            };

            for (const auto& expected : expectedMap)
            {
                auto actual = JSONHelpers::TypeFromLayoutId(expected.second);
                Assert::AreEqual(static_cast<int>(expected.first), static_cast<int>(actual));
            }
        }
    };

    TEST_CLASS (CanvasLayoutInfoUnitTests)
    {
        json::JsonObject m_json = json::JsonObject::Parse(L"{\"ref-width\": 123, \"ref-height\": 321, \"zones\": [{\"X\": 11, \"Y\": 22, \"width\": 33, \"height\": 44}, {\"X\": 55, \"Y\": 66, \"width\": 77, \"height\": 88}]}");

        TEST_METHOD (ToJson)
        {
            CanvasLayoutInfo info;
            info.lastWorkAreaWidth = 123;
            info.lastWorkAreaHeight = 321;
            info.zones = { CanvasLayoutInfo::Rect{ 11, 22, 33, 44 }, CanvasLayoutInfo::Rect{ 55, 66, 77, 88 } };

            auto actual = CanvasLayoutInfo::ToJson(info);
            compareJsonObjects(m_json, actual);
        }

        TEST_METHOD (FromJson)
        {
            CanvasLayoutInfo expected;
            expected.lastWorkAreaWidth = 123;
            expected.lastWorkAreaHeight = 321;
            expected.zones = { CanvasLayoutInfo::Rect{ 11, 22, 33, 44 }, CanvasLayoutInfo::Rect{ 55, 66, 77, 88 } };

            auto actual = CanvasLayoutInfo::FromJson(m_json);
            Assert::IsTrue(actual.has_value());

            Assert::AreEqual(expected.lastWorkAreaHeight, actual->lastWorkAreaHeight);
            Assert::AreEqual(expected.lastWorkAreaWidth, actual->lastWorkAreaWidth);
            Assert::AreEqual(expected.zones.size(), actual->zones.size());
            for (int i = 0; i < expected.zones.size(); i++)
            {
                Assert::AreEqual(expected.zones[i].x, actual->zones[i].x);
                Assert::AreEqual(expected.zones[i].y, actual->zones[i].y);
                Assert::AreEqual(expected.zones[i].width, actual->zones[i].width);
                Assert::AreEqual(expected.zones[i].height, actual->zones[i].height);
            }
        }

        TEST_METHOD (FromJsonMissingKeys)
        {
            CanvasLayoutInfo info{ 123, 321, { CanvasLayoutInfo::Rect{ 11, 22, 33, 44 }, CanvasLayoutInfo::Rect{ 55, 66, 77, 88 } } };
            const auto json = CanvasLayoutInfo::ToJson(info);

            auto iter = json.First();
            while (iter.HasCurrent())
            {
                json::JsonObject modifiedJson = json::JsonObject::Parse(json.Stringify());
                modifiedJson.Remove(iter.Current().Key());

                auto actual = CanvasLayoutInfo::FromJson(modifiedJson);
                Assert::IsFalse(actual.has_value());

                iter.MoveNext();
            }
        }

        TEST_METHOD (FromJsonInvalidTypes)
        {
            json::JsonObject m_json = json::JsonObject::Parse(L"{\"ref-width\": true, \"ref-height\": \"string\", \"zones\": [{\"X\": \"11\", \"Y\": \"22\", \"width\": \".\", \"height\": \"*\"}, {\"X\": null, \"Y\": {}, \"width\": [], \"height\": \"абвгд\"}]}");
            Assert::IsFalse(CanvasLayoutInfo::FromJson(m_json).has_value());
        }
    };

    TEST_CLASS (GridLayoutInfoUnitTests)
    {
        GridLayoutInfo m_info = GridLayoutInfo(GridLayoutInfo::Minimal{ .rows = 3, .columns = 4 });
        json::JsonObject m_gridJson = json::JsonObject();
        json::JsonArray m_rowsArray, m_columnsArray, m_cells;

        void compareSizes(int expectedRows, int expectedColumns, const GridLayoutInfo& actual)
        {
            Assert::AreEqual(expectedRows, actual.rows());
            Assert::AreEqual(expectedColumns, actual.columns());
            Assert::AreEqual((size_t)expectedRows, actual.rowsPercents().size());
            Assert::AreEqual((size_t)expectedColumns, actual.columnsPercents().size());
            Assert::AreEqual((size_t)expectedRows, actual.cellChildMap().size());

            for (int i = 0; i < expectedRows; i++)
            {
                Assert::AreEqual((size_t)expectedColumns, actual.cellChildMap()[i].size());
            }
        }

        void compareVectors(const std::vector<int>& expected, const std::vector<int>& actual)
        {
            Assert::AreEqual(expected.size(), actual.size());
            for (int i = 0; i < expected.size(); i++)
            {
                Assert::AreEqual(expected[i], actual[i]);
            }
        }

        void compareGridInfos(const GridLayoutInfo& expected, const GridLayoutInfo& actual)
        {
            compareSizes(expected.rows(), expected.columns(), actual);

            compareVectors(expected.rowsPercents(), actual.rowsPercents());
            compareVectors(expected.columnsPercents(), actual.columnsPercents());
            for (int i = 0; i < expected.cellChildMap().size(); i++)
            {
                compareVectors(expected.cellChildMap()[i], actual.cellChildMap()[i]);
            }
        }

        TEST_METHOD_INITIALIZE(Init)
            {
                m_info = GridLayoutInfo(GridLayoutInfo::Minimal{ .rows = 3, .columns = 4 });
                for (int i = 0; i < m_info.rows(); i++)
                {
                    int row = rand() % 100;
                    m_rowsArray.Append(json::JsonValue::CreateNumberValue(row));
                    m_info.rowsPercents()[i] = row;
                }

                for (int i = 0; i < m_info.columns(); i++)
                {
                    int column = rand() % 100;
                    m_columnsArray.Append(json::JsonValue::CreateNumberValue(column));
                    m_info.columnsPercents()[i] = column;
                }

                for (int i = 0; i < m_info.rows(); i++)
                {
                    json::JsonArray cellsArray;
                    for (int j = 0; j < m_info.columns(); j++)
                    {
                        int cell = rand() % 100;
                        m_info.cellChildMap()[i][j] = cell;
                        cellsArray.Append(json::JsonValue::CreateNumberValue(cell));
                    }
                    m_cells.Append(cellsArray);
                }

                m_gridJson = json::JsonObject::Parse(L"{\"rows\": 3, \"columns\": 4}");
                m_gridJson.SetNamedValue(L"rows-percentage", m_rowsArray);
                m_gridJson.SetNamedValue(L"columns-percentage", m_columnsArray);
                m_gridJson.SetNamedValue(L"cell-child-map", m_cells);
            }

            TEST_METHOD_CLEANUP(Cleanup)
                {
                    m_rowsArray.Clear();
                    m_cells.Clear();
                    m_columnsArray.Clear();
                    m_gridJson.Clear();
                    m_info = GridLayoutInfo(GridLayoutInfo::Minimal{ .rows = 3, .columns = 4 });
                }

                TEST_METHOD (CreationZero)
                {
                    const int expectedRows = 0, expectedColumns = 0;
                    GridLayoutInfo info(GridLayoutInfo::Minimal{ .rows = expectedRows, .columns = expectedColumns });
                    compareSizes(expectedRows, expectedColumns, info);
                }

                TEST_METHOD (Creation)
                {
                    const int expectedRows = 3, expectedColumns = 4;
                    const std::vector<int> expectedRowsPercents = { 0, 0, 0 };
                    const std::vector<int> expectedColumnsPercents = { 0, 0, 0, 0 };

                    GridLayoutInfo info(GridLayoutInfo::Minimal{ .rows = expectedRows, .columns = expectedColumns });
                    compareSizes(expectedRows, expectedColumns, info);

                    compareVectors(expectedRowsPercents, info.rowsPercents());
                    compareVectors(expectedColumnsPercents, info.columnsPercents());
                    for (int i = 0; i < info.cellChildMap().size(); i++)
                    {
                        compareVectors({ 0, 0, 0, 0 }, info.cellChildMap()[i]);
                    }
                }

                TEST_METHOD (CreationFull)
                {
                    const int expectedRows = 3, expectedColumns = 4;
                    const std::vector<int> expectedRowsPercents = { 1, 2, 3 };
                    const std::vector<int> expectedColumnsPercents = { 4, 3, 2, 1 };
                    const std::vector<std::vector<int>> expectedCells = { expectedColumnsPercents, expectedColumnsPercents, expectedColumnsPercents };

                    GridLayoutInfo info(GridLayoutInfo::Full{
                        .rows = expectedRows,
                        .columns = expectedColumns,
                        .rowsPercents = expectedRowsPercents,
                        .columnsPercents = expectedColumnsPercents,
                        .cellChildMap = expectedCells });
                    compareSizes(expectedRows, expectedColumns, info);

                    compareVectors(expectedRowsPercents, info.rowsPercents());
                    compareVectors(expectedColumnsPercents, info.columnsPercents());
                    for (int i = 0; i < info.cellChildMap().size(); i++)
                    {
                        compareVectors(expectedCells[i], info.cellChildMap()[i]);
                    }
                }

                TEST_METHOD (CreationFullVectorsSmaller)
                {
                    const int expectedRows = 3, expectedColumns = 4;
                    const std::vector<int> expectedRowsPercents = { 1, 2, 0 };
                    const std::vector<int> expectedColumnsPercents = { 4, 3, 0, 0 };
                    const std::vector<std::vector<int>> expectedCells = { { 0, 0, 0, 0 }, { 1, 0, 0, 0 }, { 1, 2, 0, 0 } };

                    GridLayoutInfo info(GridLayoutInfo::Full{
                        .rows = expectedRows,
                        .columns = expectedColumns,
                        .rowsPercents = { 1, 2 },
                        .columnsPercents = { 4, 3 },
                        .cellChildMap = { {}, { 1 }, { 1, 2 } } });
                    compareSizes(expectedRows, expectedColumns, info);

                    compareVectors(expectedRowsPercents, info.rowsPercents());
                    compareVectors(expectedColumnsPercents, info.columnsPercents());
                    for (int i = 0; i < info.cellChildMap().size(); i++)
                    {
                        compareVectors(expectedCells[i], info.cellChildMap()[i]);
                    }
                }

                TEST_METHOD (CreationFullVectorsBigger)
                {
                    const int expectedRows = 3, expectedColumns = 4;
                    const std::vector<int> expectedRowsPercents = { 1, 2, 3 };
                    const std::vector<int> expectedColumnsPercents = { 4, 3, 2, 1 };
                    const std::vector<std::vector<int>> expectedCells = { expectedColumnsPercents, expectedColumnsPercents, expectedColumnsPercents };

                    GridLayoutInfo info(GridLayoutInfo::Full{
                        .rows = expectedRows,
                        .columns = expectedColumns,
                        .rowsPercents = { 1, 2, 3, 4, 5 },
                        .columnsPercents = { 4, 3, 2, 1, 0, -1 },
                        .cellChildMap = { { 4, 3, 2, 1, 0, -1 }, { 4, 3, 2, 1, 0, -1 }, { 4, 3, 2, 1, 0, -1 } } });
                    compareSizes(expectedRows, expectedColumns, info);

                    compareVectors(expectedRowsPercents, info.rowsPercents());
                    compareVectors(expectedColumnsPercents, info.columnsPercents());
                    for (int i = 0; i < info.cellChildMap().size(); i++)
                    {
                        compareVectors(expectedCells[i], info.cellChildMap()[i]);
                    }
                }

                TEST_METHOD (ToJson)
                {
                    json::JsonObject expected = json::JsonObject(m_gridJson);
                    GridLayoutInfo info = m_info;

                    auto actual = GridLayoutInfo::ToJson(info);
                    compareJsonObjects(expected, actual);
                }

                TEST_METHOD (FromJson)
                {
                    json::JsonObject json = json::JsonObject(m_gridJson);
                    GridLayoutInfo expected = m_info;

                    auto actual = GridLayoutInfo::FromJson(json);
                    Assert::IsTrue(actual.has_value());
                    compareGridInfos(expected, *actual);
                }

                TEST_METHOD (FromJsonEmptyArray)
                {
                    json::JsonObject json = json::JsonObject::Parse(L"{\"rows\": 0, \"columns\": 0}");
                    GridLayoutInfo expected(GridLayoutInfo::Minimal{ 0, 0 });

                    json.SetNamedValue(L"rows-percentage", json::JsonArray());
                    json.SetNamedValue(L"columns-percentage", json::JsonArray());
                    json.SetNamedValue(L"cell-child-map", json::JsonArray());

                    auto actual = GridLayoutInfo::FromJson(json);
                    Assert::IsTrue(actual.has_value());
                    compareGridInfos(expected, *actual);
                }

                TEST_METHOD (FromJsonSmallerArray)
                {
                    GridLayoutInfo expected = m_info;
                    expected.rowsPercents().pop_back();
                    expected.columnsPercents().pop_back();
                    expected.cellChildMap().pop_back();
                    expected.cellChildMap()[0].pop_back();
                    json::JsonObject json = GridLayoutInfo::ToJson(expected);

                    auto actual = GridLayoutInfo::FromJson(json);
                    Assert::IsFalse(actual.has_value());
                }

                TEST_METHOD (FromJsonBiggerArray)
                {
                    GridLayoutInfo expected = m_info;

                    //extra
                    for (int i = 0; i < 5; i++)
                    {
                        expected.rowsPercents().push_back(rand() % 100);
                        expected.columnsPercents().push_back(rand() % 100);
                        expected.cellChildMap().push_back({});

                        for (int j = 0; j < 5; j++)
                        {
                            expected.cellChildMap()[i].push_back(rand() % 100);
                        }
                    }

                    auto json = GridLayoutInfo::ToJson(expected);

                    auto actual = GridLayoutInfo::FromJson(json);
                    Assert::IsFalse(actual.has_value());
                }

                TEST_METHOD (FromJsonMissingKeys)
                {
                    GridLayoutInfo info = m_info;
                    const auto json = json::JsonObject(m_gridJson);

                    auto iter = json.First();
                    while (iter.HasCurrent())
                    {
                        json::JsonObject modifiedJson = json::JsonObject::Parse(json.Stringify());
                        modifiedJson.Remove(iter.Current().Key());

                        auto actual = GridLayoutInfo::FromJson(modifiedJson);
                        Assert::IsFalse(actual.has_value());

                        iter.MoveNext();
                    }
                }

                TEST_METHOD (FromJsonInvalidTypes)
                {
                    json::JsonObject gridJson = json::JsonObject::Parse(L"{\"rows\": \"три\", \"columns\": \"четыре\"}");
                    Assert::IsFalse(GridLayoutInfo::FromJson(gridJson).has_value());
                }
    };

    TEST_CLASS (CustomZoneSetUnitTests)
    {
        TEST_METHOD (ToJsonGrid)
        {
            CustomZoneSetJSON zoneSet{ L"uuid", CustomZoneSetData{ L"name", CustomLayoutType::Grid, GridLayoutInfo(GridLayoutInfo::Minimal{}) } };

            json::JsonObject expected = json::JsonObject::Parse(L"{\"uuid\": \"uuid\", \"name\": \"name\", \"type\": \"grid\"}");
            expected.SetNamedValue(L"info", GridLayoutInfo::ToJson(std::get<GridLayoutInfo>(zoneSet.data.info)));

            auto actual = CustomZoneSetJSON::ToJson(zoneSet);
            compareJsonObjects(expected, actual);
        }

        TEST_METHOD (ToJsonCanvas)
        {
            CustomZoneSetJSON zoneSet{ L"uuid", CustomZoneSetData{ L"name", CustomLayoutType::Canvas, CanvasLayoutInfo{} } };

            json::JsonObject expected = json::JsonObject::Parse(L"{\"uuid\": \"uuid\", \"name\": \"name\", \"type\": \"canvas\"}");
            expected.SetNamedValue(L"info", CanvasLayoutInfo::ToJson(std::get<CanvasLayoutInfo>(zoneSet.data.info)));

            auto actual = CustomZoneSetJSON::ToJson(zoneSet);
            compareJsonObjects(expected, actual);
        }

        TEST_METHOD (FromJsonGrid)
        {
            const auto grid = GridLayoutInfo(GridLayoutInfo::Full{ 1, 3, { 10000 }, { 2500, 5000, 2500 }, { { 0, 1, 2 } } });
            CustomZoneSetJSON expected{ L"{33A2B101-06E0-437B-A61E-CDBECF502906}", CustomZoneSetData{ L"name", CustomLayoutType::Grid, grid } };

            json::JsonObject json = json::JsonObject::Parse(L"{\"uuid\": \"{33A2B101-06E0-437B-A61E-CDBECF502906}\", \"name\": \"name\", \"type\": \"grid\"}");
            json.SetNamedValue(L"info", GridLayoutInfo::ToJson(std::get<GridLayoutInfo>(expected.data.info)));

            auto actual = CustomZoneSetJSON::FromJson(json);
            Assert::IsTrue(actual.has_value());

            Assert::AreEqual(expected.uuid.c_str(), actual->uuid.c_str());
            Assert::AreEqual(expected.data.name.c_str(), actual->data.name.c_str());
            Assert::AreEqual((int)expected.data.type, (int)actual->data.type);

            auto expectedGrid = std::get<GridLayoutInfo>(expected.data.info);
            auto actualGrid = std::get<GridLayoutInfo>(actual->data.info);
            Assert::AreEqual(expectedGrid.rows(), actualGrid.rows());
            Assert::AreEqual(expectedGrid.columns(), actualGrid.columns());
        }

        TEST_METHOD (FromJsonCanvas)
        {
            CustomZoneSetJSON expected{ L"{33A2B101-06E0-437B-A61E-CDBECF502906}", CustomZoneSetData{ L"name", CustomLayoutType::Canvas, CanvasLayoutInfo{ 2, 1 } } };

            json::JsonObject json = json::JsonObject::Parse(L"{\"uuid\": \"{33A2B101-06E0-437B-A61E-CDBECF502906}\", \"name\": \"name\", \"type\": \"canvas\"}");
            json.SetNamedValue(L"info", CanvasLayoutInfo::ToJson(std::get<CanvasLayoutInfo>(expected.data.info)));

            auto actual = CustomZoneSetJSON::FromJson(json);
            Assert::IsTrue(actual.has_value());

            Assert::AreEqual(expected.uuid.c_str(), actual->uuid.c_str());
            Assert::AreEqual(expected.data.name.c_str(), actual->data.name.c_str());
            Assert::AreEqual((int)expected.data.type, (int)actual->data.type);

            auto expectedGrid = std::get<CanvasLayoutInfo>(expected.data.info);
            auto actualGrid = std::get<CanvasLayoutInfo>(actual->data.info);
            Assert::AreEqual(expectedGrid.lastWorkAreaWidth, actualGrid.lastWorkAreaWidth);
            Assert::AreEqual(expectedGrid.lastWorkAreaHeight, actualGrid.lastWorkAreaHeight);
        }

        TEST_METHOD (FromJsonGridInvalidUuid)
        {
            const auto grid = GridLayoutInfo(GridLayoutInfo::Full{ 1, 3, { 10000 }, { 2500, 5000, 2500 }, { { 0, 1, 2 } } });
            CustomZoneSetJSON expected{ L"uuid", CustomZoneSetData{ L"name", CustomLayoutType::Grid, grid } };

            json::JsonObject json = json::JsonObject::Parse(L"{\"uuid\": \"uuid\", \"name\": \"name\", \"type\": \"grid\"}");
            json.SetNamedValue(L"info", GridLayoutInfo::ToJson(std::get<GridLayoutInfo>(expected.data.info)));

            auto actual = CustomZoneSetJSON::FromJson(json);
            Assert::IsFalse(actual.has_value());
        }

        TEST_METHOD (FromJsonCanvasInvalidUuid)
        {
            CustomZoneSetJSON expected{ L"uuid", CustomZoneSetData{ L"name", CustomLayoutType::Canvas, CanvasLayoutInfo{ 2, 1 } } };

            json::JsonObject json = json::JsonObject::Parse(L"{\"uuid\": \"uuid\", \"name\": \"name\", \"type\": \"canvas\"}");
            json.SetNamedValue(L"info", CanvasLayoutInfo::ToJson(std::get<CanvasLayoutInfo>(expected.data.info)));

            auto actual = CustomZoneSetJSON::FromJson(json);
            Assert::IsFalse(actual.has_value());
        }

        TEST_METHOD (FromJsonMissingKeys)
        {
            CustomZoneSetJSON zoneSet{ L"uuid", CustomZoneSetData{ L"name", CustomLayoutType::Canvas, CanvasLayoutInfo{ 2, 1 } } };
            const auto json = CustomZoneSetJSON::ToJson(zoneSet);

            auto iter = json.First();
            while (iter.HasCurrent())
            {
                json::JsonObject modifiedJson = json::JsonObject::Parse(json.Stringify());
                modifiedJson.Remove(iter.Current().Key());

                auto actual = CustomZoneSetJSON::FromJson(modifiedJson);
                Assert::IsFalse(actual.has_value());

                iter.MoveNext();
            }
        }

        TEST_METHOD (FromJsonInvalidTypes)
        {
            json::JsonObject json = json::JsonObject::Parse(L"{\"uuid\": null, \"name\": \"имя\", \"type\": true}");
            Assert::IsFalse(CustomZoneSetJSON::FromJson(json).has_value());

            json = json::JsonObject::Parse(L"{\"uuid\": \"uuid\", \"name\": \"name\", \"type\": \"unknown type\"}");
            Assert::IsFalse(CustomZoneSetJSON::FromJson(json).has_value());
        }
    };

    TEST_CLASS (ZoneSetDataUnitTest)
    {
        TEST_METHOD (ToJsonGeneral)
        {
            json::JsonObject expected = json::JsonObject::Parse(L"{\"uuid\": \"{33A2B101-06E0-437B-A61E-CDBECF502906}\", \"type\": \"rows\"}");
            ZoneSetData data{ L"{33A2B101-06E0-437B-A61E-CDBECF502906}", ZoneSetLayoutType::Rows };
            const auto actual = ZoneSetData::ToJson(data);
            compareJsonObjects(expected, actual);
        }

        TEST_METHOD (FromJsonGeneral)
        {
            ZoneSetData expected{ L"{33A2B101-06E0-437B-A61E-CDBECF502906}", ZoneSetLayoutType::Columns };

            json::JsonObject json = json::JsonObject::Parse(L"{\"uuid\": \"{33A2B101-06E0-437B-A61E-CDBECF502906}\", \"type\": \"columns\"}");
            auto actual = ZoneSetData::FromJson(json);
            Assert::IsTrue(actual.has_value());

            Assert::AreEqual(expected.uuid.c_str(), actual->uuid.c_str());
            Assert::AreEqual((int)expected.type, (int)actual->type);
        }

        TEST_METHOD (FromJsonTypeInvalid)
        {
            ZoneSetData expected{ L"{33A2B101-06E0-437B-A61E-CDBECF502906}", ZoneSetLayoutType::Blank };

            json::JsonObject json = json::JsonObject::Parse(L"{\"uuid\": \"{33A2B101-06E0-437B-A61E-CDBECF502906}\", \"type\": \"invalid_type\"}");
            auto actual = ZoneSetData::FromJson(json);
            Assert::IsTrue(actual.has_value());

            Assert::AreEqual(expected.uuid.c_str(), actual->uuid.c_str());
            Assert::AreEqual((int)expected.type, (int)actual->type);
        }

        TEST_METHOD (FromJsonUuidInvalid)
        {
            json::JsonObject json = json::JsonObject::Parse(L"{\"uuid\": \"uuid\", \"type\": \"invalid_type\"}");
            auto actual = ZoneSetData::FromJson(json);
            Assert::IsFalse(actual.has_value());
        }

        TEST_METHOD (FromJsonMissingKeys)
        {
            ZoneSetData data{ L"{33A2B101-06E0-437B-A61E-CDBECF502906}", ZoneSetLayoutType::Columns };
            const auto json = ZoneSetData::ToJson(data);

            auto iter = json.First();
            while (iter.HasCurrent())
            {
                json::JsonObject modifiedJson = json::JsonObject::Parse(json.Stringify());
                modifiedJson.Remove(iter.Current().Key());

                auto actual = ZoneSetData::FromJson(modifiedJson);
                Assert::IsFalse(actual.has_value());

                iter.MoveNext();
            }
        }
    };

    TEST_CLASS (AppZoneHistoryUnitTests)
    {
        TEST_METHOD (ToJson)
        {
            AppZoneHistoryData data{
                .zoneSetUuid = L"zoneset-uuid", .deviceId = L"device-id", .zoneIndexSet = { 54321 }
            };
            AppZoneHistoryJSON appZoneHistory{ L"appPath", std::vector<AppZoneHistoryData>{ data } };
            json::JsonObject expected = json::JsonObject::Parse(L"{\"app-path\": \"appPath\", \"history\":[{\"zone-index-set\": [54321], \"device-id\": \"device-id\", \"zoneset-uuid\": \"zoneset-uuid\"}]}");

            auto actual = AppZoneHistoryJSON::ToJson(appZoneHistory);
            compareJsonObjects(expected, actual);
        }

        TEST_METHOD (FromJson)
        {
            AppZoneHistoryData data{
                .zoneSetUuid = L"{33A2B101-06E0-437B-A61E-CDBECF502906}", .deviceId = L"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1539}", .zoneIndexSet = { 54321 }
            };
            AppZoneHistoryJSON expected{ L"appPath", std::vector<AppZoneHistoryData>{ data } };
            json::JsonObject json = json::JsonObject::Parse(L"{\"app-path\": \"appPath\", \"history\": [{\"device-id\": \"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1539}\", \"zoneset-uuid\": \"{33A2B101-06E0-437B-A61E-CDBECF502906}\", \"zone-index\": 54321}]}");

            auto actual = AppZoneHistoryJSON::FromJson(json);
            Assert::IsTrue(actual.has_value());

            Assert::AreEqual(expected.appPath.c_str(), actual->appPath.c_str());
            Assert::AreEqual(expected.data.size(), actual->data.size());
            Assert::AreEqual(expected.data[0].zoneIndexSet, actual->data[0].zoneIndexSet);
            Assert::AreEqual(expected.data[0].deviceId.c_str(), actual->data[0].deviceId.c_str());
            Assert::AreEqual(expected.data[0].zoneSetUuid.c_str(), actual->data[0].zoneSetUuid.c_str());
        }

        TEST_METHOD (FromJsonInvalidUuid)
        {
            json::JsonObject json = json::JsonObject::Parse(L"{\"app-path\": \"appPath\", \"history\": [{\"device-id\": \"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1539}\", \"zoneset-uuid\": \"zoneset-uuid\", \"zone-index\": 54321}]}");
            auto actual = AppZoneHistoryJSON::FromJson(json);
            Assert::IsFalse(actual.has_value());
        }

        TEST_METHOD (FromJsonInvalidDeviceId)
        {
            json::JsonObject json = json::JsonObject::Parse(L"{\"app-path\": \"appPath\", \"history\": [{\"device-id\": \"device-id\", \"zoneset-uuid\": \"{33A2B101-06E0-437B-A61E-CDBECF502906}\", \"zone-index\": 54321}]}");
            auto actual = AppZoneHistoryJSON::FromJson(json);
            Assert::IsFalse(actual.has_value());
        }

        TEST_METHOD (FromJsonMissingKeys)
        {
            AppZoneHistoryData data{
                .zoneSetUuid = L"zoneset-uuid", .deviceId = L"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1539}", .zoneIndexSet = { 54321 }
            };
            AppZoneHistoryJSON appZoneHistory{ L"appPath", std::vector<AppZoneHistoryData>{ data } };
            const auto json = AppZoneHistoryJSON::ToJson(appZoneHistory);

            auto iter = json.First();
            while (iter.HasCurrent())
            {
                json::JsonObject modifiedJson = json::JsonObject::Parse(json.Stringify());
                modifiedJson.Remove(iter.Current().Key());

                auto actual = AppZoneHistoryJSON::FromJson(modifiedJson);
                Assert::IsFalse(actual.has_value());

                iter.MoveNext();
            }
        }

        TEST_METHOD (FromJsonInvalidTypes)
        {
            json::JsonObject json = json::JsonObject::Parse(L"{\"app-path\": false, \"history\": [{\"device-id\": [], \"zoneset-uuid\": {}, \"zone-index\": \"54321\"}]}");
            Assert::IsFalse(AppZoneHistoryJSON::FromJson(json).has_value());
        }

        TEST_METHOD (ToJsonMultipleDesktopAppHistory)
        {
            AppZoneHistoryData data1{
                .zoneSetUuid = L"zoneset-uuid1", .deviceId = L"device-id1", .zoneIndexSet = { 54321 }
            };
            AppZoneHistoryData data2{
                .zoneSetUuid = L"zoneset-uuid2", .deviceId = L"device-id2", .zoneIndexSet = { 12345 }
            };
            AppZoneHistoryJSON appZoneHistory{
                L"appPath", std::vector<AppZoneHistoryData>{ data1, data2 }
            };
            json::JsonObject expected = json::JsonObject::Parse(L"{\"app-path\": \"appPath\", \"history\": [{\"zone-index-set\": [54321], \"device-id\": \"device-id1\", \"zoneset-uuid\": \"zoneset-uuid1\"}, {\"zone-index-set\": [12345], \"device-id\": \"device-id2\", \"zoneset-uuid\": \"zoneset-uuid2\"}]}");

            auto actual = AppZoneHistoryJSON::ToJson(appZoneHistory);
            std::wstring s = actual.Stringify().c_str();
            compareJsonObjects(expected, actual);
        }

        TEST_METHOD (FromJsonMultipleDesktopAppHistory)
        {
            AppZoneHistoryData data1{
                .zoneSetUuid = L"{33A2B101-06E0-437B-A61E-CDBECF502906}", .deviceId = L"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1539}", .zoneIndexSet = { 54321 }
            };
            AppZoneHistoryData data2{
                .zoneSetUuid = L"{33A2B101-06E0-437B-A61E-CDBECF502906}", .deviceId = L"AOC2460#4&fe3a015&0&UID65793_1920_1200_{8a0b9205-6128-45a2-934a-b97f5b271235}", .zoneIndexSet = { 12345 }
            };
            AppZoneHistoryJSON expected{
                L"appPath", std::vector<AppZoneHistoryData>{ data1, data2 }
            };
            json::JsonObject json = json::JsonObject::Parse(L"{\"app-path\": \"appPath\", \"history\": [{\"device-id\": \"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1539}\", \"zoneset-uuid\": \"{33A2B101-06E0-437B-A61E-CDBECF502906}\", \"zone-index-set\": [54321]}, {\"device-id\": \"AOC2460#4&fe3a015&0&UID65793_1920_1200_{8a0b9205-6128-45a2-934a-b97f5b271235}\", \"zoneset-uuid\": \"{33A2B101-06E0-437B-A61E-CDBECF502906}\", \"zone-index-set\": [12345]}]}");

            auto actual = AppZoneHistoryJSON::FromJson(json);
            Assert::IsTrue(actual.has_value());

            Assert::AreEqual(expected.appPath.c_str(), actual->appPath.c_str());
            Assert::AreEqual(expected.data.size(), actual->data.size());

            for (size_t i = 0; i < expected.data.size(); ++i)
            {
                Assert::AreEqual(expected.data[i].zoneIndexSet, actual->data[i].zoneIndexSet);
                Assert::AreEqual(expected.data[i].deviceId.c_str(), actual->data[i].deviceId.c_str());
                Assert::AreEqual(expected.data[i].zoneSetUuid.c_str(), actual->data[i].zoneSetUuid.c_str());
            }
        }
    };

    TEST_CLASS (DeviceInfoUnitTests)
    {
    private:
        const std::wstring m_defaultDeviceId = L"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1539}";
        DeviceInfoJSON m_defaultDeviceInfo = DeviceInfoJSON{ m_defaultDeviceId, DeviceInfoData{ ZoneSetData{ L"{33A2B101-06E0-437B-A61E-CDBECF502906}", ZoneSetLayoutType::Custom }, true, 16, 3 } };
        json::JsonObject m_defaultJson = json::JsonObject::Parse(L"{\"device-id\": \"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1539}\", \"active-zoneset\": {\"type\": \"custom\", \"uuid\": \"{33A2B101-06E0-437B-A61E-CDBECF502906}\"}, \"editor-show-spacing\": true, \"editor-spacing\": 16, \"editor-zone-count\": 3}");

    public:
        TEST_METHOD (ToJson)
        {
            DeviceInfoJSON deviceInfo = m_defaultDeviceInfo;
            json::JsonObject expected = m_defaultJson;

            auto actual = DeviceInfoJSON::ToJson(deviceInfo);
            compareJsonObjects(expected, actual);
        }

        TEST_METHOD (FromJson)
        {
            DeviceInfoJSON expected = m_defaultDeviceInfo;
            expected.data.spacing = true;

            json::JsonObject json = DeviceInfoJSON::ToJson(expected);
            auto actual = DeviceInfoJSON::FromJson(json);
            Assert::IsTrue(actual.has_value());

            Assert::AreEqual(expected.deviceId.c_str(), actual->deviceId.c_str(), L"device id");
            Assert::AreEqual(expected.data.zoneCount, actual->data.zoneCount, L"zone count");
            Assert::AreEqual((int)expected.data.activeZoneSet.type, (int)actual->data.activeZoneSet.type, L"zone set type");
            Assert::AreEqual(expected.data.activeZoneSet.uuid.c_str(), actual->data.activeZoneSet.uuid.c_str(), L"zone set uuid");
        }

        TEST_METHOD (FromJsonSpacingTrue)
        {
            DeviceInfoJSON expected = m_defaultDeviceInfo;
            expected.data.spacing = true;

            json::JsonObject json = DeviceInfoJSON::ToJson(expected);
            auto actual = DeviceInfoJSON::FromJson(json);
            Assert::IsTrue(actual.has_value());

            Assert::AreEqual(expected.data.spacing, actual->data.spacing);
        }

        TEST_METHOD (FromJsonSpacingFalse)
        {
            DeviceInfoJSON expected = m_defaultDeviceInfo;
            expected.data.activeZoneSet.type = ZoneSetLayoutType::Custom;

            json::JsonObject json = DeviceInfoJSON::ToJson(expected);
            auto actual = DeviceInfoJSON::FromJson(json);
            Assert::IsTrue(actual.has_value());

            Assert::AreEqual(expected.data.spacing, actual->data.spacing);
        }

        TEST_METHOD (FromJsonZoneGeneral)
        {
            DeviceInfoJSON expected = m_defaultDeviceInfo;
            expected.data.activeZoneSet.type = ZoneSetLayoutType::PriorityGrid;

            json::JsonObject json = DeviceInfoJSON::ToJson(expected);
            auto actual = DeviceInfoJSON::FromJson(json);
            Assert::IsTrue(actual.has_value());

            Assert::AreEqual((int)expected.data.activeZoneSet.type, (int)actual->data.activeZoneSet.type, L"zone set type");
        }

        TEST_METHOD (FromJsonMissingKeys)
        {
            DeviceInfoJSON deviceInfo{ m_defaultDeviceId, DeviceInfoData{ ZoneSetData{ L"{33A2B101-06E0-437B-A61E-CDBECF502906}", ZoneSetLayoutType::Custom }, true, 16, 3 } };
            const auto json = DeviceInfoJSON::ToJson(deviceInfo);

            auto iter = json.First();
            while (iter.HasCurrent())
            {
                json::JsonObject modifiedJson = json::JsonObject::Parse(json.Stringify());
                modifiedJson.Remove(iter.Current().Key());

                auto actual = DeviceInfoJSON::FromJson(modifiedJson);
                Assert::IsFalse(actual.has_value());

                iter.MoveNext();
            }
        }

        TEST_METHOD (FromJsonInvalidTypes)
        {
            json::JsonObject json = json::JsonObject::Parse(L"{\"device-id\": true, \"active-zoneset\": {\"type\": null, \"uuid\": \"{33A2B101-06E0-437B-A61E-CDBECF502906}\"}, \"editor-show-spacing\": true, \"editor-spacing\": 16, \"editor-zone-count\": 3}");
            Assert::IsFalse(DeviceInfoJSON::FromJson(json).has_value());
        }

        TEST_METHOD (FromJsonInvalidUuid)
        {
            json::JsonObject json = json::JsonObject::Parse(L"{\"device-id\": \"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1539}\", \"active-zoneset\": {\"type\": \"custom\", \"uuid\": \"uuid\"}, \"editor-show-spacing\": true, \"editor-spacing\": 16, \"editor-zone-count\": 3}");
            Assert::IsFalse(DeviceInfoJSON::FromJson(json).has_value());
        }

        TEST_METHOD (FromJsonInvalidDeviceId)
        {
            json::JsonObject json = json::JsonObject::Parse(L"{\"device-id\": true, \"active-zoneset\": {\"type\": \"custom\", \"uuid\": \"{33A2B101-06E0-437B-A61E-CDBECF502906}\"}, \"editor-show-spacing\": true, \"editor-spacing\": 16, \"editor-zone-count\": 3}");
            Assert::IsFalse(DeviceInfoJSON::FromJson(json).has_value());
        }
    };

    TEST_CLASS (FancyZonesDataUnitTests)
    {
    private:
        const std::wstring_view m_moduleName = L"FancyZonesUnitTests";
        const std::wstring m_defaultDeviceId = L"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1539}";
        const std::wstring m_defaultCustomDeviceStr = L"{\"device-id\": \"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1539}\", \"active-zoneset\": {\"type\": \"custom\", \"uuid\": \"{33A2B101-06E0-437B-A61E-CDBECF502906}\"}, \"editor-show-spacing\": true, \"editor-spacing\": 16, \"editor-zone-count\": 3}";
        const json::JsonValue m_defaultCustomDeviceValue = json::JsonValue::Parse(m_defaultCustomDeviceStr);
        const json::JsonObject m_defaultCustomDeviceObj = json::JsonObject::Parse(m_defaultCustomDeviceStr);

        HINSTANCE m_hInst{};
        FancyZonesData& m_fzData = FancyZonesDataInstance();

        void compareJsonArrays(const json::JsonArray& expected, const json::JsonArray& actual)
        {
            Assert::AreEqual(expected.Size(), actual.Size());
            for (uint32_t i = 0; i < expected.Size(); i++)
            {
                compareJsonObjects(expected.GetObjectAt(i), actual.GetObjectAt(i));
            }
        }

        TEST_METHOD_INITIALIZE(Init)
            {
                m_hInst = (HINSTANCE)GetModuleHandleW(nullptr);
                m_fzData.clear_data();
            }

        TEST_METHOD_CLEANUP(CleanUp)
        {    
            std::filesystem::remove_all(PTSettingsHelper::get_module_save_folder_location(m_moduleName));
        }

        public:
            TEST_METHOD (FancyZonesDataPath)
            {
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                Assert::IsFalse(data.GetPersistFancyZonesJSONPath().empty());
            }

            TEST_METHOD (FancyZonesDataJsonEmpty)
            {
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                const auto jsonPath = data.GetPersistFancyZonesJSONPath();
                auto savedJson = json::from_file(jsonPath);

                if (std::filesystem::exists(jsonPath))
                {
                    std::filesystem::remove(jsonPath);
                }

                json::JsonObject expected;
                auto actual = data.GetPersistFancyZonesJSON();

                Assert::AreEqual(expected.Stringify().c_str(), actual.Stringify().c_str());

                if (savedJson)
                {
                    json::to_file(jsonPath, *savedJson);
                }
            }

            TEST_METHOD (FancyZonesDataJson)
            {
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                const auto jsonPath = data.GetPersistFancyZonesJSONPath();
                const auto appZoneHistoryPath = data.GetPersistAppZoneHistoryFilePath();
                auto savedJson = json::from_file(jsonPath);
                auto savedAppZoneHistory = json::from_file(appZoneHistoryPath);

                if (std::filesystem::exists(jsonPath))
                {
                    std::filesystem::remove(jsonPath);
                }

                json::JsonObject expected = json::JsonObject::Parse(L"{\"fancy-zones\":{\"custom-zonesets \":[{\"uuid\":\"uuid1\",\"name\":\"Custom1\",\"type\":\"custom\" }] }, \"app-zone-history\":[] }");
                json::to_file(jsonPath, expected);

                auto actual = data.GetPersistFancyZonesJSON();
                Assert::AreEqual(expected.Stringify().c_str(), actual.Stringify().c_str());

                if (savedJson)
                {
                    json::to_file(jsonPath, *savedJson);
                }
                else
                {
                    std::filesystem::remove(jsonPath);
                }

                if (savedAppZoneHistory)
                {
                    json::to_file(appZoneHistoryPath, *savedAppZoneHistory);
                }
                else
                {
                    std::filesystem::remove(appZoneHistoryPath);
                }
            }

            TEST_METHOD (FancyZonesDataDeviceInfoMap)
            {
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                const auto actual = data.GetDeviceInfoMap();
                Assert::IsTrue(actual.empty());
            }

            TEST_METHOD (FancyZonesDataDeviceInfoMapParseEmpty)
            {
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);

                json::JsonObject json;
                data.ParseDeviceInfos(json);

                const auto actual = data.GetDeviceInfoMap();
                Assert::IsTrue(actual.empty());
            }

            TEST_METHOD (FancyZonesDataDeviceInfoMapParseValidEmpty)
            {
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);

                json::JsonObject expected;
                json::JsonArray zoneSets;
                expected.SetNamedValue(L"devices", zoneSets);

                data.ParseDeviceInfos(expected);

                const auto actual = data.GetDeviceInfoMap();
                Assert::IsTrue(actual.empty());
            }

            TEST_METHOD (FancyZonesDataDeviceInfoMapParseInvalid)
            {
                json::JsonArray devices;
                devices.Append(json::JsonObject::Parse(m_defaultCustomDeviceStr));
                devices.Append(json::JsonObject::Parse(L"{\"device-id\": \"device_id\"}"));

                json::JsonObject expected;
                expected.SetNamedValue(L"devices", devices);

                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                auto actual = data.ParseDeviceInfos(expected);

                Assert::IsFalse(actual);
            }

            TEST_METHOD (FancyZonesDataDeviceInfoMapParseSingle)
            {
                json::JsonArray devices;
                devices.Append(m_defaultCustomDeviceValue);
                json::JsonObject expected;
                expected.SetNamedValue(L"devices", devices);

                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                data.ParseDeviceInfos(expected);

                const auto actualMap = data.GetDeviceInfoMap();
                Assert::AreEqual((size_t)1, actualMap.size());
            }

            TEST_METHOD (FancyZonesDataDeviceInfoMapParseMany)
            {
                json::JsonArray devices;
                for (int i = 0; i < 10; i++)
                {
                    json::JsonObject obj = json::JsonObject::Parse(m_defaultCustomDeviceStr);
                    obj.SetNamedValue(L"device-id", json::JsonValue::CreateStringValue(std::to_wstring(i) + L"_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1539}"));
                    devices.Append(obj);
                }

                json::JsonObject expected;
                expected.SetNamedValue(L"devices", devices);
                Logger::WriteMessage(expected.Stringify().c_str());
                Logger::WriteMessage("\n");

                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                data.ParseDeviceInfos(expected);

                const auto actualMap = data.GetDeviceInfoMap();
                Assert::AreEqual((size_t)10, actualMap.size());
            }

            TEST_METHOD (FancyZonesDataSerialize)
            {
                json::JsonArray expectedDevices;
                expectedDevices.Append(m_defaultCustomDeviceObj);
                json::JsonObject expected;
                expected.SetNamedValue(L"devices", expectedDevices);

                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                data.ParseDeviceInfos(expected);

                auto actual = data.SerializeDeviceInfos();
                compareJsonArrays(expectedDevices, actual);
            }

            TEST_METHOD (DeviceInfoSaveTemp)
            {
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                DeviceInfoJSON deviceInfo{ L"default_device_id", DeviceInfoData{ ZoneSetData{ L"uuid", ZoneSetLayoutType::Custom }, true, 16, 3 } };

                const std::wstring path = data.GetPersistFancyZonesJSONPath() + L".test_tmp";
                data.SerializeDeviceInfoToTmpFile(deviceInfo, path);

                bool actualFileExists = std::filesystem::exists(path);
                Assert::IsTrue(actualFileExists);

                auto expectedData = DeviceInfoJSON::ToJson(deviceInfo);
                auto actualSavedData = json::from_file(path);
                std::filesystem::remove(path); //clean up before compare asserts

                Assert::IsTrue(actualSavedData.has_value());
                compareJsonObjects(expectedData, *actualSavedData);
            }

            TEST_METHOD (DeviceInfoReadTemp)
            {
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                const std::wstring deviceId = m_defaultDeviceId;
                DeviceInfoJSON expected{ deviceId, DeviceInfoData{ ZoneSetData{ L"{33A2B101-06E0-437B-A61E-CDBECF502906}", ZoneSetLayoutType::Custom }, true, 16, 3 } };
                const std::wstring path = data.GetPersistFancyZonesJSONPath() + L".test_tmp";
                data.SerializeDeviceInfoToTmpFile(expected, path);

                data.ParseDeviceInfoFromTmpFile(path);

                bool actualFileExists = std::filesystem::exists(path);
                if (actualFileExists)
                {
                    std::filesystem::remove(path); //clean up before compare asserts
                }
                Assert::IsFalse(actualFileExists);

                auto devices = data.GetDeviceInfoMap();
                Assert::AreEqual((size_t)1, devices.size());

                auto actual = devices.find(deviceId)->second;
                Assert::AreEqual(expected.data.showSpacing, actual.showSpacing);
                Assert::AreEqual(expected.data.spacing, actual.spacing);
                Assert::AreEqual(expected.data.zoneCount, actual.zoneCount);
                Assert::AreEqual((int)expected.data.activeZoneSet.type, (int)actual.activeZoneSet.type);
                Assert::AreEqual(expected.data.activeZoneSet.uuid.c_str(), actual.activeZoneSet.uuid.c_str());
            }

            TEST_METHOD (DeviceInfoReadTempNonexistent)
            {
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                const std::wstring path = data.GetPersistFancyZonesJSONPath() + L".test_tmp";
                data.ParseDeviceInfoFromTmpFile(path);

                auto devices = data.GetDeviceInfoMap();
                Assert::AreEqual((size_t)0, devices.size());
            }

            TEST_METHOD (AppZoneHistoryParseSingle)
            {
                const std::wstring expectedAppPath = L"appPath";
                const std::wstring expectedDeviceId = m_defaultDeviceId;
                const std::wstring expectedZoneSetId = L"{33A2B101-06E0-437B-A61E-CDBECF502906}";
                const int expectedIndex = 54321;

                AppZoneHistoryData data{
                    .zoneSetUuid = expectedZoneSetId, .deviceId = expectedDeviceId, .zoneIndexSet = { expectedIndex }
                };
                AppZoneHistoryJSON expected{ expectedAppPath, std::vector<AppZoneHistoryData>{ data } };
                json::JsonArray zoneHistoryArray;
                zoneHistoryArray.Append(AppZoneHistoryJSON::ToJson(expected));
                json::JsonObject json;
                json.SetNamedValue(L"app-zone-history", json::JsonValue::Parse(zoneHistoryArray.Stringify()));

                FancyZonesData fancyZonesData;
                fancyZonesData.SetSettingsModulePath(m_moduleName);
                fancyZonesData.ParseAppZoneHistory(json);

                const auto actualProcessHistoryMap = fancyZonesData.GetAppZoneHistoryMap();
                Assert::AreEqual((size_t)zoneHistoryArray.Size(), actualProcessHistoryMap.size());

                const auto actualProcessHistory = actualProcessHistoryMap.begin();
                Assert::AreEqual(expectedAppPath.c_str(), actualProcessHistory->first.c_str());

                const auto actualAppZoneHistory = actualProcessHistory->second;
                Assert::AreEqual(expected.data.size(), actualAppZoneHistory.size());
                Assert::AreEqual(expectedZoneSetId.c_str(), actualAppZoneHistory[0].zoneSetUuid.c_str());
                Assert::AreEqual(expectedDeviceId.c_str(), actualAppZoneHistory[0].deviceId.c_str());
                Assert::AreEqual({ expectedIndex }, actualAppZoneHistory[0].zoneIndexSet);
            }

            TEST_METHOD (AppZoneHistoryParseManyApps)
            {
                json::JsonObject json;
                json::JsonArray zoneHistoryArray;
                AppZoneHistoryData data1{
                    .zoneSetUuid = L"{33A2B101-06E0-437B-A61E-CDBECF502900}", .deviceId = L"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1530}", .zoneIndexSet = { 1 }
                };
                AppZoneHistoryData data2{
                    .zoneSetUuid = L"{33A2B101-06E0-437B-A61E-CDBECF502901}", .deviceId = L"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1531}", .zoneIndexSet = { 2 }
                };
                AppZoneHistoryData data3{
                    .zoneSetUuid = L"{33A2B101-06E0-437B-A61E-CDBECF502902}", .deviceId = L"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1532}", .zoneIndexSet = { 3 }
                };
                AppZoneHistoryData data4{
                    .zoneSetUuid = L"{33A2B101-06E0-437B-A61E-CDBECF502903}", .deviceId = L"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1533}", .zoneIndexSet = { 4 }
                };
                zoneHistoryArray.Append(AppZoneHistoryJSON::ToJson(AppZoneHistoryJSON{ L"app-path-1", std::vector<AppZoneHistoryData>{ data1 } }));
                zoneHistoryArray.Append(AppZoneHistoryJSON::ToJson(AppZoneHistoryJSON{ L"app-path-2", std::vector<AppZoneHistoryData>{ data2 } }));
                zoneHistoryArray.Append(AppZoneHistoryJSON::ToJson(AppZoneHistoryJSON{ L"app-path-3", std::vector<AppZoneHistoryData>{ data3 } }));
                zoneHistoryArray.Append(AppZoneHistoryJSON::ToJson(AppZoneHistoryJSON{ L"app-path-4", std::vector<AppZoneHistoryData>{ data4 } }));

                json.SetNamedValue(L"app-zone-history", json::JsonValue::Parse(zoneHistoryArray.Stringify()));

                FancyZonesData fancyZonesData;
                fancyZonesData.SetSettingsModulePath(m_moduleName);
                fancyZonesData.ParseAppZoneHistory(json);

                const auto actualProcessHistoryMap = fancyZonesData.GetAppZoneHistoryMap();
                Assert::AreEqual((size_t)zoneHistoryArray.Size(), actualProcessHistoryMap.size());

                auto iter = zoneHistoryArray.First();
                while (iter.HasCurrent())
                {
                    auto expected = AppZoneHistoryJSON::FromJson(json::JsonObject::Parse(iter.Current().Stringify()));

                    const auto& actual = actualProcessHistoryMap.at(expected->appPath);
                    Assert::AreEqual(expected->data.size(), actual.size());
                    Assert::AreEqual(expected->data[0].deviceId.c_str(), actual[0].deviceId.c_str());
                    Assert::AreEqual(expected->data[0].zoneSetUuid.c_str(), actual[0].zoneSetUuid.c_str());
                    Assert::AreEqual(expected->data[0].zoneIndexSet, actual[0].zoneIndexSet);

                    iter.MoveNext();
                }
            }

            TEST_METHOD (AppZoneHistoryParseManyZonesForSingleApp)
            {
                json::JsonObject json;
                json::JsonArray zoneHistoryArray;

                const auto appPath = L"app-path";
                AppZoneHistoryData data1{
                    .zoneSetUuid = L"{33A2B101-06E0-437B-A61E-CDBECF502900}", .deviceId = L"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1530}", .zoneIndexSet = { 1 }
                };
                zoneHistoryArray.Append(AppZoneHistoryJSON::ToJson(AppZoneHistoryJSON{ appPath, std::vector<AppZoneHistoryData>{ data1 } }));
                AppZoneHistoryData data2{
                    .zoneSetUuid = L"{33A2B101-06E0-437B-A61E-CDBECF502901}", .deviceId = L"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1531}", .zoneIndexSet = { 2 }
                };
                zoneHistoryArray.Append(AppZoneHistoryJSON::ToJson(AppZoneHistoryJSON{ appPath, std::vector<AppZoneHistoryData>{ data2 } }));
                AppZoneHistoryData data3{
                    .zoneSetUuid = L"{33A2B101-06E0-437B-A61E-CDBECF502902}", .deviceId = L"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1532}", .zoneIndexSet = { 3 }
                };
                zoneHistoryArray.Append(AppZoneHistoryJSON::ToJson(AppZoneHistoryJSON{ appPath, std::vector<AppZoneHistoryData>{ data3 } }));
                AppZoneHistoryData expected{
                    .zoneSetUuid = L"{33A2B101-06E0-437B-A61E-CDBECF502903}", .deviceId = L"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1533}", .zoneIndexSet = { 4 }
                };
                zoneHistoryArray.Append(AppZoneHistoryJSON::ToJson(AppZoneHistoryJSON{ appPath, std::vector<AppZoneHistoryData>{ expected } }));
                json.SetNamedValue(L"app-zone-history", json::JsonValue::Parse(zoneHistoryArray.Stringify()));

                FancyZonesData fancyZonesData;
                fancyZonesData.SetSettingsModulePath(m_moduleName);
                fancyZonesData.ParseAppZoneHistory(json);

                const auto& actualProcessHistoryMap = fancyZonesData.GetAppZoneHistoryMap();
                Assert::AreEqual((size_t)1, actualProcessHistoryMap.size());

                const auto& actual = actualProcessHistoryMap.at(appPath);
                Assert::AreEqual((size_t)1, actual.size());
                Assert::AreEqual(expected.deviceId.c_str(), actual[0].deviceId.c_str());
                Assert::AreEqual(expected.zoneSetUuid.c_str(), actual[0].zoneSetUuid.c_str());
                Assert::AreEqual(expected.zoneIndexSet, actual[0].zoneIndexSet);
            }

            TEST_METHOD (AppZoneHistoryParseEmpty)
            {
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                data.ParseAppZoneHistory(json::JsonObject());

                auto actual = data.GetAppZoneHistoryMap();
                Assert::IsTrue(actual.empty());
            }

            TEST_METHOD (AppZoneHistoryParseInvalid)
            {
                const std::wstring appPath = L"appPath";
                json::JsonObject json;
                AppZoneHistoryData data{
                    .zoneSetUuid = L"{33A2B101-06E0-437B-A61E-CDBECF502906}", .deviceId = L"device-id", .zoneIndexSet = { 54321 }
                };
                AppZoneHistoryJSON expected{ appPath, std::vector<AppZoneHistoryData>{ data } };
                json.SetNamedValue(L"app-zone-history", json::JsonValue::Parse(AppZoneHistoryJSON::ToJson(expected).Stringify()));

                FancyZonesData fancyZonesData;
                fancyZonesData.SetSettingsModulePath(m_moduleName);
                bool actual = fancyZonesData.ParseAppZoneHistory(json);

                Assert::IsFalse(actual);
            }

            TEST_METHOD (AppZoneHistoryParseInvalidUuid)
            {
                const std::wstring appPath = L"appPath";
                json::JsonObject json;
                AppZoneHistoryData data{
                    .zoneSetUuid = L"zoneset-uuid", .deviceId = L"device-id", .zoneIndexSet = { 54321 }
                };
                AppZoneHistoryJSON expected{ appPath, std::vector<AppZoneHistoryData>{ data } };
                json.SetNamedValue(L"app-zone-history", json::JsonValue::Parse(AppZoneHistoryJSON::ToJson(expected).Stringify()));

                FancyZonesData fancyZonesData;
                fancyZonesData.SetSettingsModulePath(m_moduleName);
                bool actual = fancyZonesData.ParseAppZoneHistory(json);

                Assert::IsFalse(actual);
            }

            TEST_METHOD (AppZoneHistorySerializeSingle)
            {
                const std::wstring appPath = L"appPath";
                json::JsonArray expected;
                AppZoneHistoryData data{
                    .zoneSetUuid = L"{39B25DD2-130D-4B5D-8851-4791D66B1539}", .deviceId = m_defaultDeviceId, .zoneIndexSet = { 54321 }
                };
                AppZoneHistoryJSON appZoneHistory{
                    appPath, std::vector<AppZoneHistoryData>{ data }
                };
                expected.Append(AppZoneHistoryJSON::ToJson(appZoneHistory));
                json::JsonObject json;
                json.SetNamedValue(L"app-zone-history", json::JsonValue::Parse(expected.Stringify()));

                FancyZonesData fancyZonesData;
                fancyZonesData.SetSettingsModulePath(m_moduleName);
                fancyZonesData.ParseAppZoneHistory(json);

                auto actual = fancyZonesData.SerializeAppZoneHistory();
                compareJsonArrays(expected, actual);
            }

            TEST_METHOD (AppZoneHistorySerializeMany)
            {
                json::JsonObject json;
                json::JsonArray expected;
                AppZoneHistoryData data1{
                    .zoneSetUuid = L"{33A2B101-06E0-437B-A61E-CDBECF502906}", .deviceId = m_defaultDeviceId, .zoneIndexSet = { 54321 }
                };
                AppZoneHistoryJSON appZoneHistory1{
                    L"app-path-1", std::vector<AppZoneHistoryData>{ data1 }
                };
                AppZoneHistoryData data2{
                    .zoneSetUuid = L"{33A2B101-06E0-437B-A61E-CDBECF502906}", .deviceId = m_defaultDeviceId, .zoneIndexSet = { 54321 }
                };
                AppZoneHistoryJSON appZoneHistory2{
                    L"app-path-2", std::vector<AppZoneHistoryData>{ data2 }
                };
                AppZoneHistoryData data3{
                    .zoneSetUuid = L"{33A2B101-06E0-437B-A61E-CDBECF502906}", .deviceId = m_defaultDeviceId, .zoneIndexSet = { 54321 }
                };
                AppZoneHistoryJSON appZoneHistory3{
                    L"app-path-3", std::vector<AppZoneHistoryData>{ data3 }
                };
                AppZoneHistoryData data4{
                    .zoneSetUuid = L"{33A2B101-06E0-437B-A61E-CDBECF502906}", .deviceId = m_defaultDeviceId, .zoneIndexSet = { 54321 }
                };
                AppZoneHistoryJSON appZoneHistory4{
                    L"app-path-4", std::vector<AppZoneHistoryData>{ data4 }
                };
                expected.Append(AppZoneHistoryJSON::ToJson(appZoneHistory1));
                expected.Append(AppZoneHistoryJSON::ToJson(appZoneHistory2));
                expected.Append(AppZoneHistoryJSON::ToJson(appZoneHistory3));
                expected.Append(AppZoneHistoryJSON::ToJson(appZoneHistory4));
                json.SetNamedValue(L"app-zone-history", json::JsonValue::Parse(expected.Stringify()));

                FancyZonesData fancyZonesData;
                fancyZonesData.SetSettingsModulePath(m_moduleName);
                fancyZonesData.ParseAppZoneHistory(json);

                auto actual = fancyZonesData.SerializeAppZoneHistory();
                compareJsonArrays(expected, actual);
            }

            TEST_METHOD (AppZoneHistorySerializeEmpty)
            {
                json::JsonArray expected;
                json::JsonObject json;
                json.SetNamedValue(L"app-zone-history", json::JsonValue::Parse(expected.Stringify()));

                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                data.ParseAppZoneHistory(json);

                auto actual = data.SerializeAppZoneHistory();
                compareJsonArrays(expected, actual);
            }

            TEST_METHOD (CustomZoneSetsParseSingle)
            {
                const std::wstring zoneUuid = L"{33A2B101-06E0-437B-A61E-CDBECF502906}";
                GridLayoutInfo grid(GridLayoutInfo(JSONHelpers::GridLayoutInfo::Full{
                    .rows = 1,
                    .columns = 3,
                    .rowsPercents = { 10000 },
                    .columnsPercents = { 2500, 5000, 2500 },
                    .cellChildMap = { { 0, 1, 2 } } }));

                json::JsonObject json;
                CustomZoneSetJSON expected{ zoneUuid, CustomZoneSetData{ L"name", CustomLayoutType::Grid, grid } };
                json::JsonArray array;
                array.Append(CustomZoneSetJSON::ToJson(expected));
                json.SetNamedValue(L"custom-zone-sets", json::JsonValue::Parse(array.Stringify()));

                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                data.ParseCustomZoneSets(json);

                auto actualMap = data.GetCustomZoneSetsMap();
                Assert::AreEqual((size_t)array.Size(), actualMap.size());

                auto actual = actualMap.find(zoneUuid)->second;
                Assert::AreEqual(expected.data.name.c_str(), actual.name.c_str());
                Assert::AreEqual((int)expected.data.type, (int)actual.type);

                auto expectedGrid = std::get<GridLayoutInfo>(expected.data.info);
                auto actualGrid = std::get<GridLayoutInfo>(actual.info);
                Assert::AreEqual(expectedGrid.rows(), actualGrid.rows());
                Assert::AreEqual(expectedGrid.columns(), actualGrid.columns());
            }

            TEST_METHOD (CustomZoneSetsParseMany)
            {
                json::JsonObject json;
                json::JsonArray array;
                const GridLayoutInfo grid(GridLayoutInfo(JSONHelpers::GridLayoutInfo::Full{
                    .rows = 1,
                    .columns = 3,
                    .rowsPercents = { 10000 },
                    .columnsPercents = { 2500, 5000, 2500 },
                    .cellChildMap = { { 0, 1, 2 } } }));
                array.Append(CustomZoneSetJSON::ToJson(CustomZoneSetJSON{ L"{33A2B101-06E0-437B-A61E-CDBECF502900}", CustomZoneSetData{ L"name", CustomLayoutType::Grid, grid } }));
                array.Append(CustomZoneSetJSON::ToJson(CustomZoneSetJSON{ L"{33A2B101-06E0-437B-A61E-CDBECF502901}", CustomZoneSetData{ L"name", CustomLayoutType::Canvas, CanvasLayoutInfo{ 1, 2 } } }));
                array.Append(CustomZoneSetJSON::ToJson(CustomZoneSetJSON{ L"{33A2B101-06E0-437B-A61E-CDBECF502902}", CustomZoneSetData{ L"name", CustomLayoutType::Grid, grid } }));
                array.Append(CustomZoneSetJSON::ToJson(CustomZoneSetJSON{ L"{33A2B101-06E0-437B-A61E-CDBECF502903}", CustomZoneSetData{ L"name", CustomLayoutType::Canvas, CanvasLayoutInfo{ 1, 2 } } }));
                json.SetNamedValue(L"custom-zone-sets", json::JsonValue::Parse(array.Stringify()));

                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                data.ParseCustomZoneSets(json);

                auto actualMap = data.GetCustomZoneSetsMap();
                Assert::AreEqual((size_t)array.Size(), actualMap.size());

                auto iter = array.First();
                while (iter.HasCurrent())
                {
                    auto expected = CustomZoneSetJSON::FromJson(json::JsonObject::Parse(iter.Current().Stringify()));
                    auto actual = actualMap.find(expected->uuid)->second;
                    Assert::AreEqual(expected->data.name.c_str(), actual.name.c_str(), L"name");
                    Assert::AreEqual((int)expected->data.type, (int)actual.type, L"type");

                    if (expected->data.type == CustomLayoutType::Grid)
                    {
                        auto expectedInfo = std::get<GridLayoutInfo>(expected->data.info);
                        auto actualInfo = std::get<GridLayoutInfo>(actual.info);
                        Assert::AreEqual(expectedInfo.rows(), actualInfo.rows(), L"grid rows");
                        Assert::AreEqual(expectedInfo.columns(), actualInfo.columns(), L"grid columns");
                    }
                    else
                    {
                        auto expectedInfo = std::get<CanvasLayoutInfo>(expected->data.info);
                        auto actualInfo = std::get<CanvasLayoutInfo>(actual.info);
                        Assert::AreEqual(expectedInfo.lastWorkAreaWidth, actualInfo.lastWorkAreaWidth, L"canvas width");
                        Assert::AreEqual(expectedInfo.lastWorkAreaHeight, actualInfo.lastWorkAreaHeight, L"canvas height");
                    }

                    iter.MoveNext();
                }
            }

            TEST_METHOD (CustomZoneSetsParseEmpty)
            {
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                data.ParseCustomZoneSets(json::JsonObject());

                auto actual = data.GetCustomZoneSetsMap();
                Assert::IsTrue(actual.empty());
            }

            TEST_METHOD (CustomZoneSetsParseInvalid)
            {
                json::JsonObject json;
                CustomZoneSetJSON expected{ L"uuid", CustomZoneSetData{ L"name", CustomLayoutType::Grid, GridLayoutInfo(GridLayoutInfo::Minimal{ 1, 2 }) } };
                json.SetNamedValue(L"custom-zone-sets", json::JsonValue::Parse(CustomZoneSetJSON::ToJson(expected).Stringify()));

                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                auto actual = data.ParseCustomZoneSets(json);

                Assert::IsFalse(actual);
            }

            TEST_METHOD (CustomZoneSetsSerializeSingle)
            {
                json::JsonArray expected;
                const GridLayoutInfo grid(GridLayoutInfo(JSONHelpers::GridLayoutInfo::Full{
                    .rows = 1,
                    .columns = 3,
                    .rowsPercents = { 10000 },
                    .columnsPercents = { 2500, 5000, 2500 },
                    .cellChildMap = { { 0, 1, 2 } } }));
                expected.Append(CustomZoneSetJSON::ToJson(CustomZoneSetJSON{ L"{33A2B101-06E0-437B-A61E-CDBECF502906}", CustomZoneSetData{ L"name", CustomLayoutType::Grid, grid } }));
                json::JsonObject json;
                json.SetNamedValue(L"custom-zone-sets", json::JsonValue::Parse(expected.Stringify()));

                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                data.ParseCustomZoneSets(json);

                auto actual = data.SerializeCustomZoneSets();
                compareJsonArrays(expected, actual);
            }

            TEST_METHOD (CustomZoneSetsSerializeMany)
            {
                json::JsonObject json;
                json::JsonArray expected;
                const GridLayoutInfo grid(GridLayoutInfo(JSONHelpers::GridLayoutInfo::Full{
                    .rows = 1,
                    .columns = 3,
                    .rowsPercents = { 10000 },
                    .columnsPercents = { 2500, 5000, 2500 },
                    .cellChildMap = { { 0, 1, 2 } } }));

                expected.Append(CustomZoneSetJSON::ToJson(CustomZoneSetJSON{ L"{33A2B101-06E0-437B-A61E-CDBECF502900}", CustomZoneSetData{ L"name", CustomLayoutType::Grid, grid } }));
                expected.Append(CustomZoneSetJSON::ToJson(CustomZoneSetJSON{ L"{33A2B101-06E0-437B-A61E-CDBECF502901}", CustomZoneSetData{ L"name", CustomLayoutType::Canvas, CanvasLayoutInfo{ 1, 2 } } }));
                expected.Append(CustomZoneSetJSON::ToJson(CustomZoneSetJSON{ L"{33A2B101-06E0-437B-A61E-CDBECF502902}", CustomZoneSetData{ L"name", CustomLayoutType::Grid, grid } }));
                expected.Append(CustomZoneSetJSON::ToJson(CustomZoneSetJSON{ L"{33A2B101-06E0-437B-A61E-CDBECF502903}", CustomZoneSetData{ L"name", CustomLayoutType::Canvas, CanvasLayoutInfo{ 1, 2 } } }));
                json.SetNamedValue(L"custom-zone-sets", json::JsonValue::Parse(expected.Stringify()));

                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                data.ParseCustomZoneSets(json);

                auto actual = data.SerializeCustomZoneSets();
                compareJsonArrays(expected, actual);
            }

            TEST_METHOD (CustomZoneSetsSerializeEmpty)
            {
                json::JsonArray expected;
                json::JsonObject json;
                json.SetNamedValue(L"custom-zone-sets", json::JsonValue::Parse(expected.Stringify()));

                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                data.ParseCustomZoneSets(json);

                auto actual = data.SerializeCustomZoneSets();
                compareJsonArrays(expected, actual);
            }

            TEST_METHOD (CustomZoneSetsReadTemp)
            {
                //prepare device data
                const std::wstring deviceId = L"default_device_id";

                {
                    DeviceInfoJSON deviceInfo{ deviceId, DeviceInfoData{ ZoneSetData{ L"{33A2B101-06E0-437B-A61E-CDBECF502906}", ZoneSetLayoutType::Custom }, true, 16, 3 } };
                    const std::wstring deviceInfoPath = m_fzData.GetPersistFancyZonesJSONPath() + L".device_info_tmp";
                    m_fzData.SerializeDeviceInfoToTmpFile(deviceInfo, deviceInfoPath);

                    m_fzData.ParseDeviceInfoFromTmpFile(deviceInfoPath);
                    std::filesystem::remove(deviceInfoPath);
                }

                const std::wstring uuid = L"{33A2B101-06E0-437B-A61E-CDBECF502906}";
                const GridLayoutInfo grid(GridLayoutInfo(JSONHelpers::GridLayoutInfo::Full{
                    .rows = 1,
                    .columns = 3,
                    .rowsPercents = { 10000 },
                    .columnsPercents = { 2500, 5000, 2500 },
                    .cellChildMap = { { 0, 1, 2 } } }));
                CustomZoneSetJSON expected{ uuid, CustomZoneSetData{ L"name", CustomLayoutType::Grid, grid } };

                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                const std::wstring path = data.GetPersistFancyZonesJSONPath() + L".test_tmp";
                json::to_file(path, CustomZoneSetJSON::ToJson(expected));
                m_fzData.ParseCustomZoneSetFromTmpFile(path);

                bool actualFileExists = std::filesystem::exists(path);
                if (actualFileExists)
                {
                    std::filesystem::remove(path); //clean up before compare asserts
                }
                Assert::IsFalse(actualFileExists);

                auto devices = m_fzData.GetCustomZoneSetsMap();
                Assert::AreEqual((size_t)1, devices.size());

                auto actual = devices.find(uuid)->second;
                Assert::AreEqual((int)expected.data.type, (int)actual.type);
                Assert::AreEqual(expected.data.name.c_str(), actual.name.c_str());
                auto expectedGrid = std::get<GridLayoutInfo>(expected.data.info);
                auto actualGrid = std::get<GridLayoutInfo>(actual.info);
                Assert::AreEqual(expectedGrid.rows(), actualGrid.rows());
                Assert::AreEqual(expectedGrid.columns(), actualGrid.columns());
            }

            TEST_METHOD (CustomZoneSetsReadTempNonexistent)
            {
                const std::wstring path = m_fzData.GetPersistFancyZonesJSONPath() + L".test_tmp";
                const std::wstring deviceId = L"default_device_id";

                m_fzData.ParseCustomZoneSetFromTmpFile(path);
                auto devices = m_fzData.GetDeviceInfoMap();
                Assert::AreEqual((size_t)0, devices.size());
            }

            TEST_METHOD (SetActiveZoneSet)
            {
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                const std::wstring uniqueId = m_defaultDeviceId;

                json::JsonArray devices;
                devices.Append(m_defaultCustomDeviceValue);
                json::JsonObject json;
                json.SetNamedValue(L"devices", devices);
                data.ParseDeviceInfos(json);

                JSONHelpers::ZoneSetData expectedZoneSetData{
                    .uuid = L"{33A2B101-06E0-437B-A61E-CDBECF502906}",
                    .type = ZoneSetLayoutType::Focus
                };

                data.SetActiveZoneSet(uniqueId, expectedZoneSetData);

                auto actual = data.GetDeviceInfoMap().find(uniqueId)->second;
                Assert::AreEqual(expectedZoneSetData.uuid.c_str(), actual.activeZoneSet.uuid.c_str());
                Assert::IsTrue(expectedZoneSetData.type == actual.activeZoneSet.type);
            }

            TEST_METHOD (SetActiveZoneSetUuidEmpty)
            {
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                const std::wstring expected = L"{39B25DD2-130D-4B5D-8851-4791D66B1539}";
                const std::wstring uniqueId = m_defaultDeviceId;

                json::JsonArray devices;
                devices.Append(m_defaultCustomDeviceValue);
                json::JsonObject json;
                json.SetNamedValue(L"devices", devices);
                data.ParseDeviceInfos(json);

                JSONHelpers::ZoneSetData expectedZoneSetData{
                    .uuid = L"",
                    .type = ZoneSetLayoutType::Focus
                };

                data.SetActiveZoneSet(uniqueId, expectedZoneSetData);

                auto actual = data.GetDeviceInfoMap().find(uniqueId)->second;
                Assert::AreEqual(expectedZoneSetData.uuid.c_str(), actual.activeZoneSet.uuid.c_str());
                Assert::IsTrue(expectedZoneSetData.type == actual.activeZoneSet.type);
            }

            TEST_METHOD (SetActiveZoneSetUniqueIdInvalid)
            {
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);

                const std::wstring expected = L"{33A2B101-06E0-437B-A61E-CDBECF502906}";
                const std::wstring uniqueId = L"id-not-contained-by-device-info-map_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1539}";

                json::JsonArray devices;
                devices.Append(m_defaultCustomDeviceValue);
                json::JsonObject json;
                json.SetNamedValue(L"devices", devices);
                bool parseRes = data.ParseDeviceInfos(json);
                Assert::IsTrue(parseRes);

                JSONHelpers::ZoneSetData zoneSetData{
                    .uuid = L"new_uuid",
                    .type = ZoneSetLayoutType::Focus
                };

                data.SetActiveZoneSet(uniqueId, zoneSetData);

                const auto& deviceInfoMap = data.GetDeviceInfoMap();
                auto actual = deviceInfoMap.find(m_defaultDeviceId)->second;
                Assert::AreEqual(expected.c_str(), actual.activeZoneSet.uuid.c_str());
                Assert::IsTrue(deviceInfoMap.end() == deviceInfoMap.find(uniqueId), L"new device info should not be added");
            }

            TEST_METHOD (LoadFancyZonesDataFromJson)
            {
                FancyZonesData fancyZonesData;
                fancyZonesData.SetSettingsModulePath(m_moduleName);
                const auto jsonPath = fancyZonesData.GetPersistFancyZonesJSONPath();
                auto savedJson = json::from_file(jsonPath);

                if (std::filesystem::exists(jsonPath))
                {
                    std::filesystem::remove(jsonPath);
                }

                const GridLayoutInfo grid(GridLayoutInfo(JSONHelpers::GridLayoutInfo::Full{
                    .rows = 1,
                    .columns = 3,
                    .rowsPercents = { 10000 },
                    .columnsPercents = { 2500, 5000, 2500 },
                    .cellChildMap = { { 0, 1, 2 } } }));
                CustomZoneSetJSON zoneSets{ L"{33A2B101-06E0-437B-A61E-CDBECF502906}", CustomZoneSetData{ L"name", CustomLayoutType::Grid, grid } };
                AppZoneHistoryData data{
                    .zoneSetUuid = L"{33A2B101-06E0-437B-A61E-CDBECF502906}", .deviceId = L"device-id", .zoneIndexSet = { 54321 }
                };
                AppZoneHistoryJSON appZoneHistory{ L"app-path", std::vector<AppZoneHistoryData>{ data } };
                DeviceInfoJSON deviceInfo{ L"{33A2B101-06E0-437B-A61E-CDBECF502906}", DeviceInfoData{ ZoneSetData{ L"uuid", ZoneSetLayoutType::Custom }, true, 16, 3 } };
                json::JsonArray zoneSetsArray, appZonesArray, deviceInfoArray;
                zoneSetsArray.Append(CustomZoneSetJSON::ToJson(zoneSets));
                appZonesArray.Append(AppZoneHistoryJSON::ToJson(appZoneHistory));
                deviceInfoArray.Append(DeviceInfoJSON::ToJson(deviceInfo));
                json::JsonObject fancyZones;
                fancyZones.SetNamedValue(L"custom-zone-sets", zoneSetsArray);
                fancyZones.SetNamedValue(L"app-zone-history", appZonesArray);
                fancyZones.SetNamedValue(L"devices", deviceInfoArray);

                json::to_file(jsonPath, fancyZones);

                fancyZonesData.LoadFancyZonesData();
                if (savedJson)
                {
                    json::to_file(jsonPath, *savedJson);
                }
                else
                {
                    std::filesystem::remove(jsonPath);
                }

                Assert::IsFalse(fancyZonesData.GetCustomZoneSetsMap().empty());
                Assert::IsFalse(fancyZonesData.GetCustomZoneSetsMap().empty());
                Assert::IsFalse(fancyZonesData.GetCustomZoneSetsMap().empty());
            }

            TEST_METHOD (LoadFancyZonesDataFromCroppedJson)
            {
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);

                const auto jsonPath = data.GetPersistFancyZonesJSONPath();
                auto savedJson = json::from_file(jsonPath);

                if (std::filesystem::exists(jsonPath))
                {
                    std::filesystem::remove(jsonPath);
                }

                std::wofstream{ jsonPath.data(), std::ios::binary } << L"{ \"app-zone-history\": [], \"devices\": [{\"device-id\": \"";

                data.LoadFancyZonesData();

                if (savedJson)
                {
                    json::to_file(jsonPath, *savedJson);
                }
                else
                {
                    std::filesystem::remove(jsonPath);
                }

                Assert::IsTrue(data.GetCustomZoneSetsMap().empty());
                Assert::IsTrue(data.GetAppZoneHistoryMap().empty());
                Assert::IsTrue(data.GetDeviceInfoMap().empty());
            }

            TEST_METHOD (LoadFancyZonesDataFromJsonWithCyrillicSymbols)
            {
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                const auto jsonPath = data.GetPersistFancyZonesJSONPath();
                auto savedJson = json::from_file(jsonPath);

                if (std::filesystem::exists(jsonPath))
                {
                    std::filesystem::remove(jsonPath);
                }

                std::wofstream{ jsonPath.data(), std::ios::binary } << L"{ \"app-zone-history\": [], \"devices\": [{\"device-id\": \"кириллица\"}], \"custom-zone-sets\": []}";
                data.LoadFancyZonesData();

                if (savedJson)
                {
                    json::to_file(jsonPath, *savedJson);
                }
                else
                {
                    std::filesystem::remove(jsonPath);
                }

                Assert::IsTrue(data.GetCustomZoneSetsMap().empty());
                Assert::IsTrue(data.GetAppZoneHistoryMap().empty());
                Assert::IsTrue(data.GetDeviceInfoMap().empty());
            }

            TEST_METHOD (LoadFancyZonesDataFromJsonWithInvalidTypes)
            {
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                const auto jsonPath = data.GetPersistFancyZonesJSONPath();
                auto savedJson = json::from_file(jsonPath);

                if (std::filesystem::exists(jsonPath))
                {
                    std::filesystem::remove(jsonPath);
                }

                std::wofstream{ jsonPath.data(), std::ios::binary } << L"{ \"app-zone-history\": null, \"devices\": [{\"device-id\":\"AOC2460#4&fe3a015&0&UID65793_1920_1200_{39B25DD2-130D-4B5D-8851-4791D66B1539}\",\"active-zoneset\":{\"uuid\":\"{568EBC3A-C09C-483E-A64D-6F1F2AF4E48D}\",\"type\":\"columns\"},\"editor-show-spacing\":true,\"editor-spacing\":16,\"editor-zone-count\":3}], \"custom-zone-sets\": []}";
                data.LoadFancyZonesData();

                if (savedJson)
                {
                    json::to_file(jsonPath, *savedJson);
                }
                else
                {
                    std::filesystem::remove(jsonPath);
                }

                Assert::IsTrue(data.GetCustomZoneSetsMap().empty());
                Assert::IsTrue(data.GetAppZoneHistoryMap().empty());
                Assert::IsFalse(data.GetDeviceInfoMap().empty());
            }

            TEST_METHOD (LoadFancyZonesDataFromRegistry)
            {
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                const auto jsonPath = data.GetPersistFancyZonesJSONPath();
                auto savedJson = json::from_file(jsonPath);

                if (std::filesystem::exists(jsonPath))
                {
                    std::filesystem::remove(jsonPath);
                }

                data.LoadFancyZonesData();
                bool actual = std::filesystem::exists(jsonPath);
                if (savedJson)
                {
                    json::to_file(jsonPath, *savedJson);
                }
                else
                {
                    std::filesystem::remove(jsonPath);
                }

                Assert::IsTrue(actual);
            }

            TEST_METHOD (SaveFancyZonesData)
            {
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);
                const auto jsonPath = data.GetPersistFancyZonesJSONPath();
                auto savedJson = json::from_file(jsonPath);

                if (std::filesystem::exists(jsonPath))
                {
                    std::filesystem::remove(jsonPath);
                }

                data.SaveFancyZonesData();
                bool actual = std::filesystem::exists(jsonPath);

                if (savedJson)
                {
                    json::to_file(jsonPath, *savedJson);
                }
                else
                {
                    std::filesystem::remove(jsonPath);
                }

                Assert::IsTrue(actual);
            }

            TEST_METHOD (AppLastZoneIndex)
            {
                const std::wstring deviceId = L"device-id";
                const std::wstring zoneSetId = L"zoneset-uuid";
                const auto window = Mocks::WindowCreate(m_hInst);
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);

                Assert::AreEqual({}, data.GetAppLastZoneIndexSet(window, deviceId, zoneSetId));

                const int expectedZoneIndex = 10;
                Assert::IsTrue(data.SetAppLastZones(window, deviceId, zoneSetId, { expectedZoneIndex } ));
                Assert::AreEqual({ expectedZoneIndex }, data.GetAppLastZoneIndexSet(window, deviceId, zoneSetId));
            }

            TEST_METHOD (AppLastZoneIndexZero)
            {
                const std::wstring zoneSetId = L"zoneset-uuid";
                const std::wstring deviceId = L"device-id";
                const auto window = Mocks::WindowCreate(m_hInst);
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);

                const int expectedZoneIndex = 0;
                Assert::IsTrue(data.SetAppLastZones(window, deviceId, zoneSetId, { expectedZoneIndex }));
                Assert::AreEqual({ expectedZoneIndex }, data.GetAppLastZoneIndexSet(window, deviceId, zoneSetId));
            }

            TEST_METHOD (AppLastZoneIndexNegative)
            {
                const std::wstring zoneSetId = L"zoneset-uuid";
                const std::wstring deviceId = L"device-id";
                const auto window = Mocks::WindowCreate(m_hInst);
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);

                const int expectedZoneIndex = -1;
                Assert::IsTrue(data.SetAppLastZones(window, deviceId, zoneSetId, { expectedZoneIndex }));
                Assert::AreEqual({ expectedZoneIndex }, data.GetAppLastZoneIndexSet(window, deviceId, zoneSetId));
            }

            TEST_METHOD (AppLastZoneIndexOverflow)
            {
                const std::wstring zoneSetId = L"zoneset-uuid";
                const std::wstring deviceId = L"device-id";
                const auto window = Mocks::WindowCreate(m_hInst);
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);

                const long expectedZoneIndex = LONG_MAX;
                Assert::IsTrue(data.SetAppLastZones(window, deviceId, zoneSetId, { expectedZoneIndex }));
                Assert::AreEqual({ static_cast<int>(expectedZoneIndex) }, data.GetAppLastZoneIndexSet(window, deviceId, zoneSetId));
            }

            TEST_METHOD (AppLastZoneIndexOverride)
            {
                const std::wstring zoneSetId = L"zoneset-uuid";
                const std::wstring deviceId = L"device-id";
                const auto window = Mocks::WindowCreate(m_hInst);
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);

                const int expectedZoneIndex = 3;
                Assert::IsTrue(data.SetAppLastZones(window, deviceId, zoneSetId, { 1 }));
                Assert::IsTrue(data.SetAppLastZones(window, deviceId, zoneSetId, { 2 }));
                Assert::IsTrue(data.SetAppLastZones(window, deviceId, zoneSetId, { expectedZoneIndex }));
                Assert::AreEqual({ expectedZoneIndex }, data.GetAppLastZoneIndexSet(window, deviceId, zoneSetId));
            }

            TEST_METHOD (AppLastZoneInvalidWindow)
            {
                const std::wstring zoneSetId = L"zoneset-uuid";
                const std::wstring deviceId = L"device-id";
                const auto window = Mocks::Window();
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);

                Assert::AreEqual({}, data.GetAppLastZoneIndexSet(window, deviceId, zoneSetId));

                const int expectedZoneIndex = 1;
                Assert::IsFalse(data.SetAppLastZones(window, deviceId, zoneSetId, { expectedZoneIndex }));
            }

            TEST_METHOD (AppLastZoneNullWindow)
            {
                const std::wstring zoneSetId = L"zoneset-uuid";
                const auto window = nullptr;
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);

                const int expectedZoneIndex = 1;
                Assert::IsFalse(data.SetAppLastZones(window, L"device-id", zoneSetId, { expectedZoneIndex }));
            }

            TEST_METHOD (AppLastdeviceIdTest)
            {
                const std::wstring zoneSetId = L"zoneset-uuid";
                const std::wstring deviceId1 = L"device-id-1";
                const std::wstring deviceId2 = L"device-id-2";
                const auto window = Mocks::WindowCreate(m_hInst);
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);

                const int expectedZoneIndex = 10;
                Assert::IsTrue(data.SetAppLastZones(window, deviceId1, zoneSetId, { expectedZoneIndex }));
                Assert::AreEqual({ expectedZoneIndex }, data.GetAppLastZoneIndexSet(window, deviceId1, zoneSetId));
                Assert::AreEqual({}, data.GetAppLastZoneIndexSet(window, deviceId2, zoneSetId));
            }

            TEST_METHOD (AppLastZoneSetIdTest)
            {
                const std::wstring zoneSetId1 = L"zoneset-uuid-1";
                const std::wstring zoneSetId2 = L"zoneset-uuid-2";
                const std::wstring deviceId = L"device-id";
                const auto window = Mocks::WindowCreate(m_hInst);
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);

                const int expectedZoneIndex = 10;
                Assert::IsTrue(data.SetAppLastZones(window, deviceId, zoneSetId1, { expectedZoneIndex }));
                Assert::AreEqual({ expectedZoneIndex }, data.GetAppLastZoneIndexSet(window, deviceId, zoneSetId1));
                Assert::AreEqual({}, data.GetAppLastZoneIndexSet(window, deviceId, zoneSetId2));
            }

            TEST_METHOD (AppLastZoneRemoveWindow)
            {
                const std::wstring zoneSetId = L"zoneset-uuid";
                const std::wstring deviceId = L"device-id";
                const auto window = Mocks::WindowCreate(m_hInst);
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);

                Assert::IsTrue(data.SetAppLastZones(window, deviceId, zoneSetId, { 1 }));
                Assert::IsTrue(data.RemoveAppLastZone(window, deviceId, zoneSetId));
                Assert::AreEqual({}, data.GetAppLastZoneIndexSet(window, deviceId, zoneSetId));
            }

            TEST_METHOD (AppLastZoneRemoveUnknownWindow)
            {
                const std::wstring zoneSetId = L"zoneset-uuid";
                const std::wstring deviceId = L"device-id";
                const auto window = Mocks::WindowCreate(m_hInst);
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);

                Assert::IsFalse(data.RemoveAppLastZone(window, deviceId, zoneSetId));
                Assert::AreEqual({}, data.GetAppLastZoneIndexSet(window, deviceId, zoneSetId));
            }

            TEST_METHOD (AppLastZoneRemoveUnknownZoneSetId)
            {
                const std::wstring zoneSetIdToInsert = L"zoneset-uuid-to-insert";
                const std::wstring zoneSetIdToRemove = L"zoneset-uuid-to-remove";
                const std::wstring deviceId = L"device-id";
                const auto window = Mocks::WindowCreate(m_hInst);
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);

                Assert::IsTrue(data.SetAppLastZones(window, deviceId, zoneSetIdToInsert, { 1 }));
                Assert::IsFalse(data.RemoveAppLastZone(window, deviceId, zoneSetIdToRemove));
                Assert::AreEqual({ 1 }, data.GetAppLastZoneIndexSet(window, deviceId, zoneSetIdToInsert));
            }

            TEST_METHOD (AppLastZoneRemoveUnknownWindowId)
            {
                const std::wstring zoneSetId = L"zoneset-uuid";
                const std::wstring deviceIdToInsert = L"device-id-insert";
                const std::wstring deviceIdToRemove = L"device-id-remove";
                const auto window = Mocks::WindowCreate(m_hInst);
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);

                Assert::IsTrue(data.SetAppLastZones(window, deviceIdToInsert, zoneSetId, { 1 }));
                Assert::IsFalse(data.RemoveAppLastZone(window, deviceIdToRemove, zoneSetId));
                Assert::AreEqual({ 1 }, data.GetAppLastZoneIndexSet(window, deviceIdToInsert, zoneSetId));
            }

            TEST_METHOD (AppLastZoneRemoveNullWindow)
            {
                const std::wstring zoneSetId = L"zoneset-uuid";
                const std::wstring deviceId = L"device-id";
                const auto window = Mocks::WindowCreate(m_hInst);
                FancyZonesData data;
                data.SetSettingsModulePath(m_moduleName);

                Assert::IsFalse(data.RemoveAppLastZone(nullptr, deviceId, zoneSetId));
            }
    };
}