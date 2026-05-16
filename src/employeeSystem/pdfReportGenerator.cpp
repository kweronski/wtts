#include <wtts/pdfReportGenerator.hpp>
#include <stdexcept>
#include <iomanip>
#include <sstream>

namespace es {

void PdfReportGenerator::errorHandler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data) {
    throw std::runtime_error("Błąd libHaru: " + std::to_string(error_no) + " (szczegóły: " + std::to_string(detail_no) + ")");
}

void PdfReportGenerator::drawText(HPDF_Page page, float x, float y, const std::string& text) {
    HPDF_Page_BeginText(page);
    HPDF_Page_MoveTextPos(page, x, y);
    HPDF_Page_ShowText(page, text.c_str());
    HPDF_Page_EndText(page);
}

void PdfReportGenerator::drawLine(HPDF_Page page, float x, float y, float width) {
    HPDF_Page_SetLineWidth(page, 1.0f);
    HPDF_Page_MoveTo(page, x, y);
    HPDF_Page_LineTo(page, x + width, y);
    HPDF_Page_Stroke(page);
}

Result PdfReportGenerator::generatePayrollReport(const std::string& filepath,
                                                const PersonnelData* pd,
                                                const AttendanceData* ad) {
    if (!pd || !ad) return Result::EmployeeIsNullptrError;

    HPDF_Doc pdf = HPDF_New(errorHandler, nullptr);
    if (!pdf) return Result::AttendanceExtractionError;

    try {
        // Ustawienia strony i czcionek
        HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", nullptr);
        HPDF_Font fontBold = HPDF_GetFont(pdf, "Helvetica-Bold", nullptr);

        HPDF_Page page = HPDF_AddPage(pdf);
        HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

        float currentY = 800.0f;
        const float startX = 50.0f;

        // Nagłówek raportu
        HPDF_Page_SetFontAndSize(page, fontBold, 22);
        drawText(page, startX, currentY, "WTTS - Raport Placowy");
        currentY -= 40.0f;

        // Dane pracownika
        HPDF_Page_SetFontAndSize(page, fontBold, 12);
        drawText(page, startX, currentY, "Dane pracownika:");
        currentY -= 20.0f;

        HPDF_Page_SetFontAndSize(page, font, 11);
        drawText(page, startX, currentY, "ID: " + pd->getEmployeeId());
        currentY -= 15.0f;
        drawText(page, startX, currentY, "Imie i nazwisko: " + pd->getEmployeeName() + " " + pd->getEmployeeSurname());
        currentY -= 15.0f;
        drawText(page, startX, currentY, "Stawka: " + std::to_string(pd->getEmployeeHourlyWage()) + " PLN/h");
        currentY -= 25.0f;

        drawLine(page, startX, currentY, 500.0f);
        currentY -= 30.0f;

        // Tabela obecności
        HPDF_Page_SetFontAndSize(page, fontBold, 12);
        drawText(page, startX, currentY, "Ewidencja czasu pracy:");
        currentY -= 25.0f;

        HPDF_Page_SetFontAndSize(page, font, 10);

        double totalPay = 0.0;
        double minuteWage = static_cast<double>(pd->getEmployeeHourlyWage()) / 60.0;

        for (const auto& record : ad->getRecords()) {
            if (record.type == tu::AttendanceType::Work || record.type == tu::AttendanceType::Delivery) {
                auto minutes = record.end - record.begin;
                double earned = minutes * minuteWage;
                totalPay += earned;

                std::stringstream ss;
                ss << record.begin.year << "-" << std::setw(2) << std::setfill('0') << record.begin.month << "-"
                   << std::setw(2) << std::setfill('0') << record.begin.day << " | Czas: "
                   << minutes << " min | Wyplata: " << std::fixed << std::setprecision(2) << earned << " PLN";

                drawText(page, startX, currentY, ss.str());
                currentY -= 18.0f;

                // Nowa strona jeśli brak miejsca
                if (currentY < 100.0f) {
                    page = HPDF_AddPage(pdf);
                    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
                    HPDF_Page_SetFontAndSize(page, font, 10);
                    currentY = 800.0f;
                }
            } else if (record.type == tu::AttendanceType::Sick) {
                std::string sickStr = std::to_string(record.begin.year) + "-" + std::to_string(record.begin.month) + "-" + std::to_string(record.begin.day) + " | Chorobowe (bezplatne)";
                drawText(page, startX, currentY, sickStr);
                currentY -= 18.0f;
            }
        }

        // Podsumowanie
        currentY -= 20.0f;
        drawLine(page, startX, currentY, 500.0f);
        currentY -= 30.0f;

        HPDF_Page_SetFontAndSize(page, fontBold, 16);
        std::stringstream totalSs;
        totalSs << "Suma do wyplaty: " << std::fixed << std::setprecision(2) << totalPay << " PLN";
        drawText(page, startX, currentY, totalSs.str());

        HPDF_SaveToFile(pdf, filepath.c_str());
        HPDF_Free(pdf);
        return Result::Success;

    } catch (const std::exception& e) {
        HPDF_Free(pdf);
        return Result::AttendanceExtractionError;
    }
}

} // namespace es