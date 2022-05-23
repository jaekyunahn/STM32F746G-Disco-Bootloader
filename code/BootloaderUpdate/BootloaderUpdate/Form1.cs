using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
// Serial Port 사용
using System.IO.Ports;
//thread 사용
using System.Threading;
//
using System.IO;

using System.Collections.Concurrent;

namespace BootloaderUpdate
{
    public partial class Form1 : Form
    {
        // 시리얼 포트 클래스 정의
        SerialPort sp;

        //int timeout_time = 0;

        // 시리얼 포트 컴포트명
        string sPortName = "";
        // 시리얼 포트 baud rate
        int iBaudRate = 0;

        // 멀티 스레딩
        Thread mainthread;

        // 현재 사용 가능 한 시리얼 포트 검색
        static string[] comlist = System.IO.Ports.SerialPort.GetPortNames();

        // 시리얼 포트로 부터 수신한 데이터
        string sGetRxData = "";
        char[] sGetRxDataByteArray = new char[1024];

        // 수신된 시리얼 포트 데이터 수신완료 여부 확인 플래그
        bool bAvailabilitySerialIncomingData = false;
        // 수신된 시리얼 포트 데이터 크기
        int iSerialIncomingDataSize = 0;

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            // Baud rate 추가
            object[] baudratelist = {
                "115200",
                "57600",
                "9600"
            };
            // combobox에 아이템 추가
            comboBoxBaudrate.Items.AddRange(baudratelist);
            // 현재 선택 표기할 아이템 선택
            comboBoxBaudrate.SelectedIndex = 0;

            //COM Port가 있는 경우에만 콤보 박스에 추가.
            if (comlist.Length > 0)
            {
                // String으로 되어 있는 목록을 콤보 박스에 추가
                comboBoxPort.Items.AddRange(comlist);
                //0번째 index 위치한 아이템 선택
                comboBoxPort.SelectedIndex = 0;
                // 현재 선택 된 시리얼포트 컴포트명을 sPortName에 적용
                sPortName = comboBoxPort.SelectedItem.ToString();
                printf("select port:" + sPortName + "\r\n");
            }
        }

        private void buttonSerialOpen_Click(object sender, EventArgs e)
        {
            //Serial Close 상태일때
            if (buttonSerialOpen.Text == "Open")
            {
                // 시리얼포트 설정 및 열기
                fSerialSettingFuncion(sPortName, iBaudRate);
                // 시리얼 포트 열기 버튼을 닫기로 변경
                buttonSerialOpen.Text = "Close";
                printf("Serial Port Open\r\n");
            }
            //Serial Open 상태일때
            else if (buttonSerialOpen.Text == "Close")
            {
                // 시리얼 포트 클래스가 등록 된 상태면
                if (null != sp)
                {
                    // 시리얼 포트가 열려 있을 때
                    if (sp.IsOpen)
                    {
                        // 시리얼 포트 닫기
                        sp.Close();
                        // 시리얼 포트 리소스 해제
                        sp.Dispose();
                        // 시리얼 포트 클래스 제거
                        sp = null;
                    }
                }
                // 시리얼포트 닫기 버튼을 열기로 변경
                buttonSerialOpen.Text = "Open";
                printf("Serial Port Close\r\n");
            }
        }

        private void buttonUpdateFileFind_Click(object sender, EventArgs e)
        {
            //파일 선택창 열기
            // OpenFileDialog 생성
            OpenFileDialog openfiledialog = new OpenFileDialog();
            //필터 조건 설정
            openfiledialog.Filter = "모든 파일 (*.*) | *.*";
            // 파일 선택창 show
            openfiledialog.ShowDialog();
            // 선택 완료시 반환된 주소 get
            textBoxUpdateFileAddress.Text = openfiledialog.FileName;
        }

        

        private void buttonUpdateStart_Click(object sender, EventArgs e)
        {
            // 각종 로그 제거
            textBoxdebug.Text = "";
            textBoxLog.Text = "";
            textBoxTerminal.Text = "";

            // Thread Start
            mainthread = new Thread(new ThreadStart(fMainThreadFunction));
            mainthread.Start();
        }

        private void buttonTest_Click(object sender, EventArgs e)
        {
#if false
            //sned_data("test\n");
            byte[] send_data = new byte[5];
            send_data[0] = (byte)('t');
            send_data[1] = (byte)('e');
            send_data[2] = (byte)('s');
            send_data[3] = (byte)('t');
            send_data[4] = (byte)('\n');
            sned_byte_array_data(send_data, 5);
#endif
        }

        private void buttonSerialPortSearch_Click(object sender, EventArgs e)
        {
            //COM Port 리스트 얻어 오기
            comlist = System.IO.Ports.SerialPort.GetPortNames();
            //COM Port가 있는 경우에만 콤보 박스에 추가.
            if (comlist.Length > 0)
            {
                comboBoxPort.Items.Clear();
                // String으로 되어 있는 목록을 콤보 박으에 추가
                comboBoxPort.Items.AddRange(comlist);
                //0번째 index 위치한 아이템 선택
                comboBoxPort.SelectedIndex = 0;
            }
        }

        private void buttonExit_Click(object sender, EventArgs e)
        {
            try
            {
                // 시리얼 포트가 열려 있으면 닫기
                if (sp.IsOpen)
                {
                    sp.Close();
                    sp.Dispose();
                    sp = null;
                }
            }
            catch
            {
                //error
            }

            try
            {
                // 메인 스레드 강제 종료
                mainthread.Abort();
            }
            catch
            {
                //error
            }
            // 현재 어플 닫기
            Application.Exit();
        }

        private void buttonstop_Click(object sender, EventArgs e)
        {
            try
            {
                // 메인 스레드 종료
                mainthread.Abort();
            }
            catch
            {
                //error
            }
        }
        /*
         *  Function
         */

        // 현재 시각
        string GetNow()
        {
            return DateTime.Now.ToString("yyyy-MM-dd:HH:mm:ss.ffff");
        }

        // Log에 찍을 printf함수
        void printf(string data)
        {
            Invoke((MethodInvoker)delegate
            {
                //textBoxEventLogBox.Text = textBoxEventLogBox.Text + "[" + GetNow() + "]" + data;
                textBoxLog.Text = textBoxLog.Text + "["+ GetNow() + "]" +data;
                //Textbox의 마지막 부분으로 커서 이동
                textBoxLog.Select(textBoxLog.Text.Length, 0);
                //Control 내용을 현재 Caret 위치까지 Scroll
                textBoxLog.ScrollToCaret();
                //현재 Control에 대한 Focues
                textBoxLog.Focus();
            });
        }

        // 시리얼 포트 수신 전용 텍스트 박스 입력
        void tprintf(string data)
        {
            Invoke((MethodInvoker)delegate
            {
                //textBoxEventLogBox.Text = textBoxEventLogBox.Text + "[" + GetNow() + "]" + data;
                textBoxTerminal.Text = textBoxTerminal.Text + data;
                //Textbox의 마지막 부분으로 커서 이동
                textBoxTerminal.Select(textBoxTerminal.Text.Length, 0);
                //Control 내용을 현재 Caret 위치까지 Scroll
                textBoxTerminal.ScrollToCaret();
                //현재 Control에 대한 Focues
                textBoxTerminal.Focus();
            });
        }

        // Serial Port Setting
        public void fSerialSettingFuncion(string sPortName, int iPortSpeed)
        {
            try
            {
                // 시리얼 포트 클래스가 비어 있는 경우
                if (null == sp)
                {
                    // 클래스 초기화
                    sp = new SerialPort();
                    // 수신 이벤트 핸들러 발생 시 동작 시킬 함수 등록
                    sp.DataReceived += new SerialDataReceivedEventHandler(sp_DataReceived);
                    // 컴포트명 설정
                    sp.PortName = sPortName;
                    // 보레이트 설정
                    sp.BaudRate = iPortSpeed;  
                    // 시리얼 포트 열기
                    sp.Open();
                }

            }
            catch (System.Exception ex)
            {
                printf(ex.Message + "\r\n");
            }
        }

        // Serial Port Received Function
        void sp_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            Invoke((MethodInvoker)delegate
            {
                // 수신 데이터 크기
                int iRecSize = sp.BytesToRead;
                // 수신 데이터 크기 만큼 byte 배열 선언 
                byte[] buff = new byte[iRecSize];
                // 수신 데이터 String type으로 저장
                string strRxData = "";
                // 수신 데이터 크기가 0이 아닐 경우
                if (iRecSize != 0)
                {
                    // 수신 데이터 가져오기
                    sp.Read(buff, 0, iRecSize);
                    // 수신 데이터 크기 누적
                    // 문자열이 길어지면 잘려서 오는 현상이 존재 하여 넣은 기능
                    iSerialIncomingDataSize += iRecSize;
                    // 수신 된 문자열 전역 변수에 저장
                    for (int iTemp = 0; iTemp < iRecSize; iTemp++)
                    {
                        //char 배열에 저장
                        sGetRxDataByteArray[iTemp] = Convert.ToChar(buff[iTemp]);
                        // 별도 수신 버퍼에 쓰기
                        textBoxdebug.Text += Convert.ToChar(buff[iTemp]);
                        // 터미널 역할 할 로그창에 쓰기 위한 작업
                        if (buff[iTemp] == '\n')
                        {
                            strRxData += '\r';
                        }
                        strRxData += Convert.ToChar(buff[iTemp]);
                    }
                    // MCU로 부터 수신된 데이터 터미널 처럼 출력하는 부분
                    tprintf(strRxData);
                    // 현재 까지 날라온 데이터 크기
                    labelincomingCount.Text = Convert.ToString(iSerialIncomingDataSize);
                    // 데이터 수신 알림
                    bAvailabilitySerialIncomingData = true;
                }

            });
        }

        void sned_data(string sendData)
        {
            try
            {
                // String Type data 시리얼 포트로 전송
                sp.Write(sendData);
            }
            catch (System.Exception ex)
            {
                printf(ex.Message + "\r\n");
            }
        }

        void sned_byte_array_data(byte[] sendData, int sendsize)
        {
            try
            {
                // byte type data 시리얼 포트로 전송
                sp.Write(sendData, 0, sendsize);
            }
            catch (System.Exception ex)
            {
                printf(ex.Message + "\r\n");
            }
        }

        void sned_byte_data(byte sendData)
        {
            // 1바이트 전송 
            try
            {
                byte[] tmp = new byte[2];
                tmp[0] = sendData;
                sp.Write(tmp, 0, 1);
            }
            catch (System.Exception ex)
            {
                printf(ex.Message + "\r\n");
            }
        }

        /*
         *  Thread
         */
        void fMainThreadFunction()
        {
            // 전송 할 파일이 등록 된 경우에만 실행
            if (textBoxUpdateFileAddress.TextLength != 0)
            {
                // while문 제어 변수
                // 스레드 전반적인 반복문 제어 변수
                bool loop = true;
                // MCU로 부터 응답을 대기 할 때 사용 할 반복문 제어 변수
                bool inloop = true;

                //범용 변수
                int x, y;

                // 현재 진행 파트
                int flow = 0;
                // 이전 진행 파트
                int beforeflow = 0;

                // XOR check sum 계산 결과 저장 하는 부분
                byte check_cum = 0;

                //최대 128MB 저장 버퍼
                byte[] filereadbuffer = new byte[134217728];

                //전송할 파일 read
                try {
                    filereadbuffer = File.ReadAllBytes(textBoxUpdateFileAddress.Text);
                }
                catch(System.Exception ex) {
                    printf("File ReadAllBytes Error" + ex.Message + "\r\n");
                    mainthread.Abort();
                }
                // 파일 크기
                int ifilesize = filereadbuffer.Length;
                // 전송 할 패킷 개수
                int paketcount = 0;
                // 전송 한 패킷 인덱스
                int pecketindex = 0;
                // packet size
                int paketsize = Convert.ToInt32(textBoxPacketSize.Text);
                // 마지막 패킷 길이
                int lastpaketdatalength = 0;
                // 전송 할 패킷 데이터 (배열) 선언
                byte[] sendbuffer = new byte[paketsize * 2];
                // filename
                string filename = "newfw.bin";

                // 파일 전송 반복문 시작
                while (loop)
                {
                    // 현재 작업 별 동작 제어 switch 
                    switch (flow)
                    {
                        // case 0 -> 파일 정보 전송 (HEAD 패킷)
                        case 0:
                            try
                            {
                                printf("send head\r\n");
                                printf("filename:" + filename + "\r\n");
                                printf("paketsize:" + paketsize + "\r\n");
                                printf("ifilesize:" + ifilesize + "\r\n");

                                // 패킷 개수, 마지막 패킷 개수 
                                paketcount = ifilesize / paketsize;
                                // 마지막 패킷 길이 확인
                                lastpaketdatalength = ifilesize % paketsize;
                                if (lastpaketdatalength > 0)
                                {
                                    paketcount++;
                                }
                                printf("paketcount:" + paketcount + "\r\n");
                                printf("lastpaketdatalength:" + lastpaketdatalength + "\r\n");

                                // 수신 버퍼 및 로그 초기화
                                textBoxdebug.Text = "";
                                sGetRxDataByteArray = new char[1024];
                                iSerialIncomingDataSize = 0;

                                // 헤더 퍀킷 전송,
                                // 파일명, 패킷 최대크기, 패킷 개수, 마지막패킷 길이, 파일크기
                                sned_data("HEAD:" + filename + "," + paketsize + "," + paketcount + "," + lastpaketdatalength + "," + ifilesize + "\n");
                                printf("HEAD:" + filename + "," + paketsize + "," + paketcount + "," + lastpaketdatalength + "," + ifilesize + "\r\n");

                                // 전송 한 패킷 개수 초기화
                                pecketindex = 0;

                                // 헤더 정보 전송 응답 대기로 이동
                                beforeflow = 0;
                                flow = 4;
                            }
                            catch (ArgumentException ex)
                            {
                                printf("case 0 error" + ex.Message + "\r\n");
                            }
                            break;
                        // case 1 -> 파일 데이터 전송
                        case 1:
                            try
                            {
                                // 현재 전송 할 패킷 인덱스 표시
                                labelNowIndex.Text = Convert.ToString(pecketindex);

                                // 수신 버퍼 비우기
                                textBoxdebug.Text = "";
                                sGetRxDataByteArray = new char[1024];
                                iSerialIncomingDataSize = 0;

                                /*
                                 *  데이터 패킷 전송
                                 */
                                // 현재 전송 현황
                                printf("pecketindex=" + (pecketindex+1) + "/" + ( paketcount) + "\r\n");
                                // 헤더
                                sned_data("DATA:");
                                // 마지막 패킷 순서일 경우
                                if ((pecketindex + 1)== paketcount)
                                {
                                    // 패킷에 쓸 데이터 시작 인덱스로 이동
                                    y = (paketsize * pecketindex);
                                    // 마지막 전송 잔여 크기만큼 복사
                                    for (x = 0; x < lastpaketdatalength; x++)
                                    {
                                        sendbuffer[x] = filereadbuffer[y];
                                        y++;
                                    }
                                    // 체크섬 계산
                                    check_cum = xor_checksum(sendbuffer, lastpaketdatalength);
                                    // 데이터 전송
                                    sned_byte_array_data(sendbuffer, lastpaketdatalength);
                                    // 체크섬 계산값 전송
                                    sned_byte_data(check_cum);
                                }
                                // 그외 순서일 경우
                                else
                                {
                                    // 패킷에 쓸 데이터 시작 인덱스로 이동
                                    y = (paketsize * pecketindex);
                                    // 패킷 크기만큼 복사
                                    for (x = 0; x < paketsize; x++)
                                    {
                                        sendbuffer[x] = filereadbuffer[y];
                                        y++;
                                    }
                                    // 체크섬 계산
                                    check_cum = xor_checksum(sendbuffer, paketsize);
                                    // 데이터 전송
                                    sned_byte_array_data(sendbuffer, paketsize);
                                    // 체크섬 계산값 전송
                                    sned_byte_data(check_cum);
                                }
                                // 패킷 마지막 문자 전송
                                sned_data("\n");
                                // 체크섬 계산값과 버퍼 첫번째 문자 출력 (디버깅 용도)
                                printf("send data:" + check_cum + "," + sendbuffer[0] + "\r\n");
                                // 응답 대기로 이동
                                beforeflow = 1;
                                flow = 4;
                            }
                            catch (ArgumentException ex)
                            {
                                printf("case 1 error" + ex.Message + "\r\n");
                                loop = false;
                            }
                            break;
                        // case 2 -> 파일 전송 끝남
                        case 2:
                            // 마무리
                            printf("case 2\r\n");

                            Thread.Sleep(1000);
                            
                            // 수신 버퍼 초기화
                            textBoxdebug.Text = "";
                            sGetRxDataByteArray = new char[1024];
                            iSerialIncomingDataSize = 0;

                            Thread.Sleep(1000);

                            // 파일전송 종료 알림
                            sned_data("END\n");

                            // 응답 대기로 이동
                            beforeflow = 2;
                            flow = 4;

                            break;
                        // case 3 -> 스레드 종료
                        case 3:
                            //종료
                            printf("case 3\r\n");
                            // 반복문 종료
                            loop = false; 
                            // 스레드 종료
                            mainthread.Abort();
                            break;
                        // case 4 -> 패킷 전송 후 MCU에서 패킷을 제대로 수신 했는지, 제대로 저장 했는지 여부에 따라 다음 단계 진행이나 패킷 재전송 할지 여부 판단
                        case 4:
                            try
                            {
                                // 수신 대기 반복문
                                inloop = true;
                                //printf("wait from MCU\r\n");
                                //타이머 동작
                                //timeout_time = 0;
                                //timeout.Start();
                                
                                // 전송한 패킷에 대한 응답 대기
                                while (inloop)
                                {
                                    // 데이터 수신이 감지 되고 수신된 패킷이 2바이트 이상인 경우 실행
                                    if (bAvailabilitySerialIncomingData && (iSerialIncomingDataSize >= 2))
                                    {
                                        // 헤더패킷에 대한 응답
                                        if (beforeflow == 0)
                                        {
                                            // 수신이 정상인 응답인 경우
                                            if (textBoxdebug.Text == "AH\n")
                                            {
                                                //다음 패킷으로 이동
                                                flow = beforeflow + 1;
                                                inloop = false;
                                            }
                                            // 수신이 비정상인 응답인 경우
                                            else if (textBoxdebug.Text == "NH\n")
                                            {
                                                // 재전송 시도
                                                flow = beforeflow ;
                                                inloop = false;
                                            }
                                        }
                                        // 데이터패킷에 대한 응답
                                        else if (beforeflow == 1)
                                        {
                                            // 수신이 정상인 응답인 경우
                                            if (textBoxdebug.Text == "AD\n")
                                            {
                                                //패킷 인덱스 증가
                                                pecketindex++;
                                                // 데이터 패킷 전송 완료 시점인 경우
                                                if (pecketindex == paketcount)
                                                {
                                                    // 다음 단계
                                                    flow = beforeflow + 1;
                                                }
                                                // 데이터 패킷 보낼 수량이 존재하면
                                                else if (pecketindex < paketcount)
                                                {
                                                    // 계속 진행
                                                    flow = beforeflow;
                                                }
                                                inloop = false;
                                            }
                                            // 수신이 비정상인 응답인 경우
                                            if (textBoxdebug.Text == "ND\n")
                                            {
                                                flow = beforeflow;
                                                inloop = false;
                                            }
                                        }
                                        // 전송종료에 대한 응답
                                        else if (beforeflow == 2)
                                        {
                                            // 전송이 완료 되면
                                            if (textBoxdebug.Text == "AE\n")
                                            {
                                                // 다음 단계로 이동
                                                flow = beforeflow + 1;
                                                inloop = false;
                                                //로그창 클리어
                                                textBoxdebug.Text = "";
                                                textBoxLog.Text = "";
                                                textBoxTerminal.Text = "";
                                            }
                                            // 종료 패킷에 대한 처리에 문제 발생하면
                                            else if (textBoxdebug.Text == "NE\n")
                                            {
                                                flow = beforeflow;
                                                inloop = false;
                                            }
                                        }
                                    }
                                }
                            }
                            catch (Exception ex)
                            {
                                printf("case 4 error" + ex.Message + "\r\n");
                                mainthread.Abort();

                            }
                            break;
                        default:
                            break;
                    }
                    Thread.Sleep(1);
                }
                mainthread.Abort();
            }
            else
            {
                printf("파일명을 입력 해 주세요\r\n");
                mainthread.Abort();
            }
        }

        private void comboBoxPort_SelectedIndexChanged(object sender, EventArgs e)
        {
            // 시리얼포트 컴포넌트 콤보 박스 변경 감지되면 sPortName에 현재 선택된 아이템 등록
            sPortName = comboBoxPort.SelectedItem.ToString();
        }

        private void comboBoxBaudrate_SelectedIndexChanged(object sender, EventArgs e)
        {
            // 시리얼포트 보레이트 콤보 박스 변경 감지되면 iBaudRate에 현재 선택된 아이템 등록
            iBaudRate = Convert.ToInt32(comboBoxBaudrate.SelectedItem.ToString());
        }

        private void timeout_Tick(object sender, EventArgs e)
        {
            //timeout_time++;
        }

        /*
         *  XOR check sum function
         */
        byte xor_checksum(byte[] data, int size)
        {
            byte res = 0x00;
            try 
            {
                for (int i = 0; i < size; i++)
                {
                    res ^= data[i];
                }
            }
            catch (Exception ex)
            {
                printf("xor checksum Error:"+ ex.Message + "\r\n");
            }
            return res;
        }
    }
}
