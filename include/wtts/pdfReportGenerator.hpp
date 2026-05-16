#pragma once

#include <string>
#include <hpdf.h>
#include <wtts/esResult.hpp>
#include <wtts/employeeData.hpp>

namespace es {

/**
 * @brief Klasa odpowiedzialna za generowanie raportów PDF przy użyciu libHaru.
 */
class PdfReportGenerator {
public:
  /**
   * @brief Generuje raport płacowy dla konkretnego pracownika.
   * * @param filepath Ścieżka docelowa pliku PDF.
   * @param pd Wskaźnik do danych osobowych pracownika.
   * @param ad Wskaźnik do danych o obecności pracownika.
   * @return Result Status operacji (Success lub błąd).
   */
  static Result generatePayrollReport(const std::string& filepath,
                                      const PersonnelData* pd,
                                      const AttendanceData* ad);

private:
  static void errorHandler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data);
  static void drawText(HPDF_Page page, float x, float y, const std::string& text);
  static void drawLine(HPDF_Page page, float x, float y, float width);
};

} // namespace es
