#define _CRT_SECURE_NO_WARNINGS 1

#include <windows.h>
#include <stdio.h>
#include "clib/cJSON.h"

#include <gdiplus.h>
#pragma comment(lib,"gdiplus.lib")
using namespace Gdiplus;

static int gdiplus_init(int b)
{
    static GdiplusStartupInput gdiplusStartupInput;
    static ULONG_PTR gdiplusToken;
    static int b_inited = 0;
    if (b == 1) {
        if (b_inited == 1)
            return 0;
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
        b_inited = 1;
    }
    else {
        GdiplusShutdown(gdiplusToken);
        b_inited = 0;
    }
    return 0;
}

static LPDEVMODE GetLandscapeDevMode(char* pDevice)
{
    HANDLE hPrinter;
    LPDEVMODE pDevMode;
    DWORD dwNeeded, dwRet;
    HWND hWnd = GetDesktopWindow();
    /* Start by opening the printer */
    if (!OpenPrinter(pDevice, &hPrinter, NULL))
        return NULL;
    /*
    * Step 1:
    * Allocate a buffer of the correct size.
    */
    dwNeeded = DocumentProperties(hWnd,
        hPrinter, /* Handle to our printer. */
        pDevice, /* Name of the printer. */
        NULL, /* Asking for size, so */
        NULL, /* these are not used. */
        0); /* Zero returns buffer size. */
    pDevMode = (LPDEVMODE)malloc(dwNeeded);

    /*
    * Step 2:
    * Get the default DevMode for the printer and
    * modify it for your needs.
    */
    dwRet = DocumentProperties(hWnd,
        hPrinter,
        pDevice,
        pDevMode, /* The address of the buffer to fill. */
        NULL, /* Not using the input buffer. */
        DM_OUT_BUFFER); /* Have the output buffer filled. */
    if (dwRet != IDOK)
    {
        /* If failure, cleanup and return failure. */
        free(pDevMode);
        ClosePrinter(hPrinter);
        return NULL;
    }

    /*
    * Make changes to the DevMode which are supported.
    */
    if (pDevMode->dmFields & DM_ORIENTATION)
    {
        /* If the printer supports paper orientation, set it.*/
        pDevMode->dmOrientation = DMORIENT_LANDSCAPE;
    }

    if (pDevMode->dmFields & DM_DUPLEX)
    {
        /* If it supports duplex printing, use it. */
        //pDevMode->dmDuplex = DMDUP_HORIZONTAL;
    }

    /*
    * Step 3:
    * Merge the new settings with the old.
    * This gives the driver an opportunity to update any private
    * portions of the DevMode structure.
    */
    dwRet = DocumentProperties(hWnd,
        hPrinter,
        pDevice,
        pDevMode, /* Reuse our buffer for output. */
        pDevMode, /* Pass the driver our changes. */
        DM_IN_BUFFER | /* Commands to Merge our changes and */
        DM_OUT_BUFFER); /* write the result. */

    /* Finished with the printer */
    ClosePrinter(hPrinter);

    if (dwRet != IDOK)
    {
        /* If failure, cleanup and return failure. */
        free(pDevMode);
        return NULL;
    }

    /* Return the modified DevMode structure. */
    return pDevMode;
}


int draw_my_logo(Graphics* graphics, float x, float y, float r)
{
    Brush* b_brush = new SolidBrush(Color(255, 0, 0, 0));
    Brush* w_brush = new SolidBrush(Color(255, 255, 255, 255));

    graphics->FillEllipse(b_brush, x-r, y-r, 2*r, 2*r);
    graphics->FillEllipse(w_brush, x , y - r/2, r, r);
    graphics->FillEllipse(b_brush, x + r / 2, y - r / 4, r/2, r/2);


    delete b_brush;
    delete w_brush;
    return 0;
}

int draw_page_stock_in(HDC hdc, float y_offset, float page_width, float page_height, cJSON* jvoucher)
{
    float page_left = 20;
    float page_top = 10;
    float page_right = 10;
    float page_bottom = 10;
    float row_points[32];
    float col_points[32];

    float line_height = (page_height - page_top - page_bottom) / 14;
    float table_width = page_width - page_left - page_right;
    for (int i = 0; i < 15; i++) {
        row_points[i] = y_offset + page_top + i * line_height;
    }
    col_points[0] = page_left;
    col_points[1] = col_points[0] + table_width * 0.06;
    col_points[2] = col_points[1] + table_width * 0.3;
    col_points[3] = col_points[2] + table_width * 0.1;
    col_points[4] = col_points[3] + table_width * 0.1;
    col_points[5] = col_points[4] + table_width * 0.08;
    col_points[6] = col_points[5] + table_width * 0.2;
    col_points[7] = col_points[6] + table_width * 0.16;

    Graphics* graphics = new Graphics(hdc);
    graphics->SetPageUnit(UnitMillimeter);
    //graphics->SetSmoothingMode();
    graphics->SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

    Pen* pen = new Pen(Color(255, 0, 0, 0), 0.1);

    for (int i = 2; i < 14; i++) {
        graphics->DrawLine(pen, col_points[0], row_points[i], col_points[7], row_points[i]);
    }
    float w = 0.6;
    graphics->DrawLine(pen, col_points[0] - w, row_points[2] - w, col_points[7] + w, row_points[2] - w);
    graphics->DrawLine(pen, col_points[0] - w, row_points[13] + w, col_points[7] + w, row_points[13] + w);

    for (int i = 0; i < 8; i++) {
        graphics->DrawLine(pen, col_points[i], row_points[2], col_points[i], row_points[13]);
    }
    graphics->DrawLine(pen, col_points[0] - w, row_points[2] - w, col_points[0] - w, row_points[13] + w);
    graphics->DrawLine(pen, col_points[7] + w, row_points[2] - w, col_points[7] + w, row_points[13] + w);

    draw_my_logo(graphics, 10.0, 15.0 + y_offset, 6.0);

    FontFamily fontFamily(L"微软雅黑");
    Font font(&fontFamily, line_height * 3 / 4, FontStyleBold, UnitMillimeter);
    Font font2(&fontFamily, line_height / 2, FontStyleRegular, UnitMillimeter);
    Brush* b_brush = new SolidBrush(Color(255, 0, 0, 0));

    StringFormat format;//创建字体格式
    format.SetAlignment(StringAlignmentCenter);//指定水平居中对齐
    format.SetLineAlignment(StringAlignmentCenter);//指定垂直居中对齐

    RectF rectf(col_points[0], row_points[0], col_points[7] - col_points[0], row_points[1] - row_points[0]);
    //graphics->DrawString(L"什么玩意", -1, &font, PointF(0, 0), b_brush);
    graphics->DrawString(L"入  库  凭  证", -1, &font, rectf, &format, b_brush);
    graphics->DrawLine(pen, col_points[0] + 60, row_points[1], col_points[7] - 60, row_points[1]);
    graphics->DrawLine(pen, col_points[0] + 60, row_points[1] + w, col_points[7] - 60, row_points[1] + w);

    const wchar_t* headers[8] = { L"N0.",L"物品名称", L"入库", L"出库", L"单位", L"摘要",L"备注", L"N0.", };
    for (int i = 0; i < 7; i++) {
        RectF rect(col_points[i], row_points[2], col_points[i + 1] - col_points[i], row_points[3] - row_points[2]);
        graphics->DrawString(headers[i], -1, &font2, rect, &format, b_brush);
    }



    StringFormat format2;//创建字体格式
    format2.SetAlignment(StringAlignmentNear);//指定水平居中对齐
    format2.SetLineAlignment(StringAlignmentCenter);//指定垂直居中对齐
    RectF rectf2(col_points[0], row_points[1], col_points[7] - col_points[0], row_points[2] - row_points[1]);
    //graphics->DrawString(L"什么玩意", -1, &font, PointF(0, 0), b_brush);
    graphics->DrawString(L"202   年     月", -1, &font2, rectf2, &format2, b_brush);
    format2.SetAlignment(StringAlignmentFar);//指定水平居中对齐
    graphics->DrawString(L"出字第        号", -1, &font2, rectf2, &format2, b_brush);

    RectF rectf_bottom(col_points[0], row_points[13], col_points[7] - col_points[0], row_points[14] - row_points[13]);
    //graphics->DrawString(L"什么玩意", -1, &font, PointF(0, 0), b_brush);
    graphics->DrawString(L"磁县阜才中学                库管员：                      后勤主任： ",
        -1, &font2, rectf_bottom, &format, b_brush);

    for (int i = 0; i < 10; i++) {
        float y1 = row_points[i + 3];
        float y2 = row_points[i + 4];
        for (int j = 0; j < 7; j++) {
            float x1 = col_points[j];
            float x2 = col_points[j+1];
            RectF rect_gird(x1, y1, x2 - x1, y2 - y1);
            graphics->DrawString(L"2", -1, &font2, rect_gird, &format, b_brush);
        }
    }


    delete b_brush;
    delete pen;
    delete graphics;
    return 0;
}

int draw_page_stock_out(HDC hdc, float y_offset,float page_width,float page_height)
{
    float page_left = 20;
    float page_top = 10;
    float page_right = 10;
    float page_bottom = 10;
    float row_points[32];
    float col_points[32];

    float line_height = (page_height - page_top - page_bottom) / 14;
    float table_width = page_width - page_left - page_right;
    for (int i = 0; i < 15; i++){
        row_points[i] = y_offset + page_top + i * line_height;
    }
    col_points[0] = page_left;
    col_points[1] = col_points[0] + table_width *0.06;
    col_points[2] = col_points[1] + table_width * 0.3;
    col_points[3] = col_points[2] + table_width * 0.07;
    col_points[4] = col_points[3] + table_width * 0.07;
    col_points[5] = col_points[4] + table_width * 0.15;
    col_points[6] = col_points[5] + table_width * 0.15;
    col_points[7] = col_points[6] + table_width * 0.2;

    Graphics* graphics = new Graphics(hdc);
    graphics->SetPageUnit(UnitMillimeter);
    //graphics->SetSmoothingMode();
    graphics->SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

    Pen* pen = new Pen(Color(255, 0, 0, 0),0.1);

    for (int i = 2; i < 14; i++) {
        graphics->DrawLine(pen, col_points[0], row_points[i], col_points[7], row_points[i]);
    }
    float w = 0.6;
    graphics->DrawLine(pen, col_points[0] - w, row_points[2] - w, col_points[7] + w, row_points[2] - w);
    graphics->DrawLine(pen, col_points[0] - w, row_points[13] + w, col_points[7] + w, row_points[13] + w);

    for (int i = 0; i < 8; i++) {
        graphics->DrawLine(pen, col_points[i], row_points[2], col_points[i], row_points[13]);
    }
    graphics->DrawLine(pen, col_points[0] -w, row_points[2] - w, col_points[0] - w, row_points[13]+ w);
    graphics->DrawLine(pen, col_points[7] + w, row_points[2] - w, col_points[7] + w, row_points[13] + w);

    draw_my_logo(graphics, 10.0, 15.0 + y_offset,6.0);

    FontFamily fontFamily(L"微软雅黑");
    Font font(&fontFamily, line_height*3/4, FontStyleBold, UnitMillimeter);
    Font font2(&fontFamily, line_height / 2, FontStyleRegular, UnitMillimeter);
    Brush* b_brush = new SolidBrush(Color(255, 0, 0, 0));
 
    StringFormat format;//创建字体格式
    format.SetAlignment(StringAlignmentCenter);//指定水平居中对齐
    format.SetLineAlignment(StringAlignmentCenter);//指定垂直居中对齐

    RectF rectf(col_points[0],row_points[0], col_points[7] - col_points[0], row_points[1] - row_points[0]);
    //graphics->DrawString(L"什么玩意", -1, &font, PointF(0, 0), b_brush);
    graphics->DrawString(L"出  库  登  记  表", -1, &font, rectf, &format, b_brush);
    graphics->DrawLine(pen, col_points[0]+60 , row_points[1] , col_points[7] - 60, row_points[1]);
    graphics->DrawLine(pen, col_points[0] +60, row_points[1] + w, col_points[7] - 60, row_points[1] + w);

    const wchar_t* headers[8] = {L"N0.",L"物品名称", L"数量", L"单位", L"领用人", L"日期", L"备注"};
    for (int i = 0; i < 7; i++) {
        RectF rect(col_points[i], row_points[2], col_points[i+1] - col_points[i], row_points[3] - row_points[2]);
        graphics->DrawString(headers[i], -1, &font2, rect, &format, b_brush);
    }



    StringFormat format2;//创建字体格式
    format2.SetAlignment(StringAlignmentNear);//指定水平居中对齐
    format2.SetLineAlignment(StringAlignmentCenter);//指定垂直居中对齐
    RectF rectf2(col_points[0], row_points[1], col_points[7] - col_points[0], row_points[2] - row_points[1]);
    //graphics->DrawString(L"什么玩意", -1, &font, PointF(0, 0), b_brush);
    graphics->DrawString(L"202   年     月", -1, &font2, rectf2, &format2, b_brush);
    format2.SetAlignment(StringAlignmentFar);//指定水平居中对齐
    graphics->DrawString(L"出字第        号", -1, &font2, rectf2, &format2, b_brush);

    RectF rectf_bottom(col_points[0], row_points[13], col_points[7] - col_points[0], row_points[14] - row_points[13]);
    //graphics->DrawString(L"什么玩意", -1, &font, PointF(0, 0), b_brush);
    graphics->DrawString(L"磁县阜才中学                库管员：                      后勤主任： ",
        -1, &font2, rectf_bottom, &format, b_brush);


    delete b_brush;
    delete pen;
    delete graphics;
    return 0;
}

extern "C" cJSON* print_voucher(cJSON* jvoucher)
{
    gdiplus_init(1);
    char print_name[1024] = "\\\\printserver\\print2";
    DWORD size = 1024;
    if (!GetDefaultPrinter(print_name, &size)){
        printf("Failure");
    }

    // Get a device context for the printer.
    HDC hdcPrint = CreateDC(NULL, print_name, NULL, NULL);

    //PRINTDLG printDlg;
    //ZeroMemory(&printDlg, sizeof(printDlg));
    //printDlg.lStructSize = sizeof(printDlg);
    //printDlg.Flags = PD_RETURNDC;

    //StartDoc(printDlg.hDC, &docInfo);
    //StartPage(printDlg.hDC);
    //Graphics* graphics = new Graphics(printDlg.hDC);

    // Display a print dialog box.
    //if (!PrintDlg(&printDlg))
    //{
    //    printf("Failure\n");
    //}

    DOCINFO docInfo;
    ZeroMemory(&docInfo, sizeof(docInfo));
    docInfo.cbSize = sizeof(docInfo);
    docInfo.lpszDocName = "GdiplusPrint";

    StartDoc(hdcPrint, &docInfo);

    double WidthsPixels = GetDeviceCaps(hdcPrint, HORZRES);
    double HeightsPixels = GetDeviceCaps(hdcPrint, VERTRES);
    double WidthsPixelsPerInch = GetDeviceCaps(hdcPrint, LOGPIXELSX);
    double HeightsPixelsPeInch = GetDeviceCaps(hdcPrint, LOGPIXELSY);
    //float  Screendpi = (FLOAT)GetDpiForWindow(GetDesktopWindow());
    float page_width = 25.4 * WidthsPixels / WidthsPixelsPerInch;
    float page_height = 25.4 * HeightsPixels / HeightsPixelsPeInch;//

    StartPage(hdcPrint);
        draw_page_stock_in(hdcPrint, 0, page_width, page_height / 2,jvoucher);
        draw_page_stock_out(hdcPrint, page_height/2,page_width, page_height / 2);
    EndPage(hdcPrint);

    EndDoc(hdcPrint);

    DeleteDC(hdcPrint);

    return 0;
}

Status gdiplus_draw_utf8(Graphics* graphics,const char* string, const Font* font, const RectF& layoutRect,
    const StringFormat* stringFormat, const Brush* brush)
{
    WCHAR wch_buf[1024] = L"";
    MultiByteToWideChar(CP_UTF8, 0, string, -1, wch_buf, 1024);
    return graphics->DrawString(wch_buf, -1, font, layoutRect, stringFormat, brush);
}


int draw_recode_page(HDC hdc,float page_width, float page_height,int page,int pages, cJSON* jrecodes)
{
    float page_left = 30;
    float page_top = 10;
    float page_right = 10;
    float page_bottom = 10;
    float row_points[64];
    float col_points[32];
    int row_num = 34;   //行数
    int col_num = 10;   //列数

    float line_height = (page_height - page_top - page_bottom) / 34;
    float table_width = page_width - page_left - page_right;
    for (int i = 0; i < row_num+1; i++) {
        row_points[i] =  page_top + i * line_height;
    }
    float col_widths[16] = {40.0,40.0,60.0,20.0,20.0,20.0,20.0,20.0,40.0,40.0};
    float col_width_total = 0.0;
    for (int i = 0; i < col_num; i++)
        col_width_total += col_widths[i];

    col_points[0] = page_left;
    for (int i = 0; i < col_num; i++) {
        col_points[i + 1] = col_points[i] + table_width * col_widths[i] / col_width_total;
    }
    Graphics* graphics = new Graphics(hdc);
    graphics->SetPageUnit(UnitMillimeter);
    //graphics->SetSmoothingMode();
    graphics->SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

    Pen* pen = new Pen(Color(255, 0, 0, 0), 0.1);
    //画装钉线
    graphics->DrawLine(pen, 15.0, 10.0, 15.0, row_points[row_num]);
    //画表格横线
    for (int i = 2; i < row_num; i++) {
        graphics->DrawLine(pen, col_points[0], row_points[i], col_points[col_num], row_points[i]);
    }
    //画上下外
    float w = 0.6; 
    graphics->DrawLine(pen, col_points[0] - w, row_points[2] - w, col_points[col_num] + w, row_points[2] - w);
    graphics->DrawLine(pen, col_points[0] - w, row_points[row_num-1] + w, col_points[col_num ] + w, row_points[row_num - 1] + w);
    //画坚线
    for (int i = 0; i <= col_num ; i++) {
        graphics->DrawLine(pen, col_points[i], row_points[2], col_points[i], row_points[row_num-1]);
    }
    graphics->DrawLine(pen, col_points[0] - w, row_points[2] - w, col_points[0] - w, row_points[row_num - 1] + w);
    graphics->DrawLine(pen, col_points[col_num] + w, row_points[2] - w, col_points[col_num] + w, row_points[row_num - 1] + w);

    draw_my_logo(graphics, 22.0, 30.0 , 6.0);

    FontFamily fontFamily(L"微软雅黑");
    Font font(&fontFamily, line_height * 3 / 4, FontStyleBold, UnitMillimeter);
    Font font2(&fontFamily, line_height / 2, FontStyleRegular, UnitMillimeter);
    Brush* b_brush = new SolidBrush(Color(255, 0, 0, 0));

    StringFormat format;//创建字体格式
    format.SetAlignment(StringAlignmentCenter);//指定水平居中对齐
    format.SetLineAlignment(StringAlignmentCenter);//指定垂直居中对齐

    RectF rectf(col_points[0], row_points[0], col_points[col_num] - col_points[0], row_points[1] - row_points[0]);
    //graphics->DrawString(L"什么玩意", -1, &font, PointF(0, 0), b_brush);
    graphics->DrawString(L"出 入  库 记 录 表", -1, &font, rectf, &format, b_brush);
    graphics->DrawLine(pen, col_points[0] + 90, row_points[1], col_points[col_num] - 90, row_points[1]);
    graphics->DrawLine(pen, col_points[0] + 90, row_points[1] + w, col_points[col_num] - 90, row_points[1] + w);

    const wchar_t* headers[10] = { L"凭证字号",L"日期",L"物品名称",L"单位", L"入库", L"出库",  L"金额",  L"库存",L"经手人", L"摘要或备注", };
    for (int i = 0; i < 10; i++) {
        RectF rect(col_points[i], row_points[2], col_points[i + 1] - col_points[i], row_points[3] - row_points[2]);
        graphics->DrawString(headers[i], -1, &font2, rect, &format, b_brush);
    }

    StringFormat format2;//创建字体格式
    format2.SetAlignment(StringAlignmentNear);//指定水平居中对齐
    format2.SetLineAlignment(StringAlignmentCenter);//指定垂直居中对齐
    RectF rectf2(col_points[0], row_points[1], col_points[col_num] - col_points[0], row_points[2] - row_points[1]);
    //graphics->DrawString(L"什么玩意", -1, &font, PointF(0, 0), b_brush);
    graphics->DrawString(L"202   年     月", -1, &font2, rectf2, &format2, b_brush);
    format2.SetAlignment(StringAlignmentFar);//指定水平居中对齐
    char text_page_info[64] = "";
    sprintf(text_page_info, "第%d页/共%d页", page+1, pages);
    gdiplus_draw_utf8(graphics, text_page_info, &font2, rectf2, &format2, b_brush);
    //graphics->DrawString(L"第  页/共  页", -1, &font2, rectf2, &format2, b_brush);

    RectF rectf_bottom(col_points[0], row_points[row_num-1], 
        col_points[col_num] - col_points[0], row_points[row_num-1] - row_points[row_num - 2]);
    //graphics->DrawString(L"什么玩意", -1, &font, PointF(0, 0), b_brush);
    graphics->DrawString(L"磁县阜才中学                         库管员：                           后勤主任： ",
        -1, &font2, rectf_bottom, &format, b_brush);

    for (int i = 0; i < 30; i++) {
        float y1 = row_points[i + 3];
        float y2 = row_points[i + 4];
        const char* row_texts[10] = { 0 };
        cJSON * jrow = cJSON_GetArrayItem(jrecodes, page * 30 + i);
        if (jrow == NULL)
            break;
        char text0[32] = "";
        char text4[32] = "";
        char text5[32] = "";
        char text6[32] = "";
        char text7[32] = "";
        sprintf(text0, "%s第%03d号", cJSON_GetObjectItem(jrow, "word")->valuestring, cJSON_GetObjectItem(jrow, "code")->valueint);
        sprintf(text4, "%d", cJSON_GetObjectItem(jrow, "stock_in")->valueint);
        sprintf(text5, "%d", cJSON_GetObjectItem(jrow, "stock_out")->valueint);
        sprintf(text6, "%d", cJSON_GetObjectItem(jrow, "stock")->valueint);
        sprintf(text7, "%d", cJSON_GetObjectItem(jrow, "amount")->valueint);
        
        row_texts[0] = text0;
        row_texts[1] = cJSON_GetObjectItem(jrow, "date")->valuestring;
        row_texts[2] = cJSON_GetObjectItem(jrow, "item_name")->valuestring;
        row_texts[3] = cJSON_GetObjectItem(jrow, "unit")->valuestring;
        row_texts[4] = text4;
        row_texts[5] = text5;
        row_texts[6] = text7;
        row_texts[7] = text6;
        row_texts[8] = cJSON_GetObjectItem(jrow, "who")->valuestring;
        row_texts[9] = cJSON_GetObjectItem(jrow, "note")->valuestring;

        for (int j = 0; j < 10; j++) {
            float x1 = col_points[j];
            float x2 = col_points[j + 1];
            RectF rect_gird(x1, y1, x2 - x1, y2 - y1);
            //graphics->DrawString(L"2", -1, &font2, rect_gird, &format, b_brush);
            const char * string = row_texts[j];
            gdiplus_draw_utf8(graphics, string, &font2, rect_gird, &format, b_brush);
        }
    }


    delete b_brush;
    delete pen;
    delete graphics;
    return 0;
}

extern "C" int print_recode(cJSON* jprint_data)
{
    gdiplus_init(1);
    char print_name[1024] = "\\\\printserver\\print2";
    DWORD size = 1024;
    if (!GetDefaultPrinter(print_name, &size)) {
        printf("Failure");
        return -1;
    }

    LPDEVMODE devMode = GetLandscapeDevMode(print_name);
    HDC hdcPrint = CreateDC(NULL, print_name, NULL, devMode);
    free(devMode);

    DOCINFO docInfo;
    ZeroMemory(&docInfo, sizeof(docInfo));
    docInfo.cbSize = sizeof(docInfo);
    docInfo.lpszDocName = "GdiplusPrint";

    StartDoc(hdcPrint, &docInfo);

    double WidthsPixels = GetDeviceCaps(hdcPrint, HORZRES);//宽度单位像素
    double HeightsPixels = GetDeviceCaps(hdcPrint, VERTRES);
    double WidthsPixelsPerInch = GetDeviceCaps(hdcPrint, LOGPIXELSX);//分辨率 像素每英寸
    double HeightsPixelsPeInch = GetDeviceCaps(hdcPrint, LOGPIXELSY);
    //float  Screendpi = (FLOAT)GetDpiForWindow(GetDesktopWindow());
    float page_width = 25.4 * WidthsPixels / WidthsPixelsPerInch;   //毫米宽度
    float page_height = 25.4 * HeightsPixels / HeightsPixelsPeInch;//

    cJSON* jrecodes = cJSON_GetObjectItem(jprint_data, "recodes");
    int recode_total = cJSON_GetArraySize(jrecodes);
    int pages = recode_total / 30;
    if (recode_total % 30 != 0) 
        pages++;
    for (int page = 0; page < pages; page++) {
        StartPage(hdcPrint);
        draw_recode_page(hdcPrint, page_width, page_height,page,pages, jrecodes);
        EndPage(hdcPrint);
    }
    EndDoc(hdcPrint);
    DeleteDC(hdcPrint);
    return 0;
}


