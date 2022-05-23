
namespace BootloaderUpdate
{
    partial class Form1
    {
        /// <summary>
        /// 필수 디자이너 변수입니다.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 사용 중인 모든 리소스를 정리합니다.
        /// </summary>
        /// <param name="disposing">관리되는 리소스를 삭제해야 하면 true이고, 그렇지 않으면 false입니다.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form 디자이너에서 생성한 코드

        /// <summary>
        /// 디자이너 지원에 필요한 메서드입니다. 
        /// 이 메서드의 내용을 코드 편집기로 수정하지 마세요.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.buttonSerialOpen = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.textBoxLog = new System.Windows.Forms.TextBox();
            this.buttonUpdateStart = new System.Windows.Forms.Button();
            this.textBoxUpdateFileAddress = new System.Windows.Forms.TextBox();
            this.buttonUpdateFileFind = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.buttonSerialPortSearch = new System.Windows.Forms.Button();
            this.comboBoxPort = new System.Windows.Forms.ComboBox();
            this.comboBoxBaudrate = new System.Windows.Forms.ComboBox();
            this.textBoxTerminal = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.buttonExit = new System.Windows.Forms.Button();
            this.buttonTest = new System.Windows.Forms.Button();
            this.buttonstop = new System.Windows.Forms.Button();
            this.labelNowIndex = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.labelflow = new System.Windows.Forms.Label();
            this.textBoxdebug = new System.Windows.Forms.TextBox();
            this.label8 = new System.Windows.Forms.Label();
            this.labelincomingCount = new System.Windows.Forms.Label();
            this.timeout = new System.Windows.Forms.Timer(this.components);
            this.textBoxPacketSize = new System.Windows.Forms.TextBox();
            this.label9 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // buttonSerialOpen
            // 
            this.buttonSerialOpen.Location = new System.Drawing.Point(196, 46);
            this.buttonSerialOpen.Name = "buttonSerialOpen";
            this.buttonSerialOpen.Size = new System.Drawing.Size(75, 49);
            this.buttonSerialOpen.TabIndex = 0;
            this.buttonSerialOpen.Text = "Open";
            this.buttonSerialOpen.UseVisualStyleBackColor = true;
            this.buttonSerialOpen.Click += new System.EventHandler(this.buttonSerialOpen_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(10, 51);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(27, 12);
            this.label1.TabIndex = 1;
            this.label1.Text = "Port";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(10, 78);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(34, 12);
            this.label2.TabIndex = 4;
            this.label2.Text = "Baud";
            // 
            // textBoxLog
            // 
            this.textBoxLog.Location = new System.Drawing.Point(279, 46);
            this.textBoxLog.MaxLength = 3276799;
            this.textBoxLog.Multiline = true;
            this.textBoxLog.Name = "textBoxLog";
            this.textBoxLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.textBoxLog.Size = new System.Drawing.Size(428, 729);
            this.textBoxLog.TabIndex = 6;
            // 
            // buttonUpdateStart
            // 
            this.buttonUpdateStart.Location = new System.Drawing.Point(14, 574);
            this.buttonUpdateStart.Name = "buttonUpdateStart";
            this.buttonUpdateStart.Size = new System.Drawing.Size(261, 63);
            this.buttonUpdateStart.TabIndex = 7;
            this.buttonUpdateStart.Text = "Start";
            this.buttonUpdateStart.UseVisualStyleBackColor = true;
            this.buttonUpdateStart.Click += new System.EventHandler(this.buttonUpdateStart_Click);
            // 
            // textBoxUpdateFileAddress
            // 
            this.textBoxUpdateFileAddress.Location = new System.Drawing.Point(14, 237);
            this.textBoxUpdateFileAddress.Name = "textBoxUpdateFileAddress";
            this.textBoxUpdateFileAddress.Size = new System.Drawing.Size(261, 21);
            this.textBoxUpdateFileAddress.TabIndex = 8;
            this.textBoxUpdateFileAddress.Text = "D:\\project\\MCU\\stm32f746_disco_application\\Debug\\stm32f746_disco_application.bin";
            // 
            // buttonUpdateFileFind
            // 
            this.buttonUpdateFileFind.Location = new System.Drawing.Point(200, 208);
            this.buttonUpdateFileFind.Name = "buttonUpdateFileFind";
            this.buttonUpdateFileFind.Size = new System.Drawing.Size(75, 23);
            this.buttonUpdateFileFind.TabIndex = 9;
            this.buttonUpdateFileFind.Text = "Find";
            this.buttonUpdateFileFind.UseVisualStyleBackColor = true;
            this.buttonUpdateFileFind.Click += new System.EventHandler(this.buttonUpdateFileFind_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(14, 213);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(73, 12);
            this.label3.TabIndex = 10;
            this.label3.Text = "Choose File";
            // 
            // buttonSerialPortSearch
            // 
            this.buttonSerialPortSearch.Location = new System.Drawing.Point(12, 17);
            this.buttonSerialPortSearch.Name = "buttonSerialPortSearch";
            this.buttonSerialPortSearch.Size = new System.Drawing.Size(259, 23);
            this.buttonSerialPortSearch.TabIndex = 11;
            this.buttonSerialPortSearch.Text = "serial port search";
            this.buttonSerialPortSearch.UseVisualStyleBackColor = true;
            this.buttonSerialPortSearch.Click += new System.EventHandler(this.buttonSerialPortSearch_Click);
            // 
            // comboBoxPort
            // 
            this.comboBoxPort.FormattingEnabled = true;
            this.comboBoxPort.Location = new System.Drawing.Point(58, 48);
            this.comboBoxPort.Name = "comboBoxPort";
            this.comboBoxPort.Size = new System.Drawing.Size(132, 20);
            this.comboBoxPort.TabIndex = 12;
            this.comboBoxPort.SelectedIndexChanged += new System.EventHandler(this.comboBoxPort_SelectedIndexChanged);
            // 
            // comboBoxBaudrate
            // 
            this.comboBoxBaudrate.FormattingEnabled = true;
            this.comboBoxBaudrate.Location = new System.Drawing.Point(58, 75);
            this.comboBoxBaudrate.Name = "comboBoxBaudrate";
            this.comboBoxBaudrate.Size = new System.Drawing.Size(132, 20);
            this.comboBoxBaudrate.TabIndex = 13;
            this.comboBoxBaudrate.SelectedIndexChanged += new System.EventHandler(this.comboBoxBaudrate_SelectedIndexChanged);
            // 
            // textBoxTerminal
            // 
            this.textBoxTerminal.Location = new System.Drawing.Point(713, 46);
            this.textBoxTerminal.MaxLength = 3276799;
            this.textBoxTerminal.Multiline = true;
            this.textBoxTerminal.Name = "textBoxTerminal";
            this.textBoxTerminal.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.textBoxTerminal.Size = new System.Drawing.Size(750, 729);
            this.textBoxTerminal.TabIndex = 14;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(277, 22);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(26, 12);
            this.label4.TabIndex = 15;
            this.label4.Text = "Log";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(804, 22);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(37, 12);
            this.label5.TabIndex = 16;
            this.label5.Text = "Serial";
            // 
            // buttonExit
            // 
            this.buttonExit.Location = new System.Drawing.Point(12, 712);
            this.buttonExit.Name = "buttonExit";
            this.buttonExit.Size = new System.Drawing.Size(261, 63);
            this.buttonExit.TabIndex = 17;
            this.buttonExit.Text = "Exit";
            this.buttonExit.UseVisualStyleBackColor = true;
            this.buttonExit.Click += new System.EventHandler(this.buttonExit_Click);
            // 
            // buttonTest
            // 
            this.buttonTest.Location = new System.Drawing.Point(12, 264);
            this.buttonTest.Name = "buttonTest";
            this.buttonTest.Size = new System.Drawing.Size(107, 49);
            this.buttonTest.TabIndex = 18;
            this.buttonTest.Text = "Test";
            this.buttonTest.UseVisualStyleBackColor = true;
            this.buttonTest.Click += new System.EventHandler(this.buttonTest_Click);
            // 
            // buttonstop
            // 
            this.buttonstop.Location = new System.Drawing.Point(14, 643);
            this.buttonstop.Name = "buttonstop";
            this.buttonstop.Size = new System.Drawing.Size(261, 63);
            this.buttonstop.TabIndex = 19;
            this.buttonstop.Text = "Stop";
            this.buttonstop.UseVisualStyleBackColor = true;
            this.buttonstop.Click += new System.EventHandler(this.buttonstop_Click);
            // 
            // labelNowIndex
            // 
            this.labelNowIndex.AutoSize = true;
            this.labelNowIndex.Location = new System.Drawing.Point(155, 138);
            this.labelNowIndex.Name = "labelNowIndex";
            this.labelNowIndex.Size = new System.Drawing.Size(11, 12);
            this.labelNowIndex.TabIndex = 20;
            this.labelNowIndex.Text = "0";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(36, 138);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(66, 12);
            this.label6.TabIndex = 21;
            this.label6.Text = "Now index";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(36, 161);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(58, 12);
            this.label7.TabIndex = 23;
            this.label7.Text = "Now flow";
            // 
            // labelflow
            // 
            this.labelflow.AutoSize = true;
            this.labelflow.Location = new System.Drawing.Point(155, 161);
            this.labelflow.Name = "labelflow";
            this.labelflow.Size = new System.Drawing.Size(11, 12);
            this.labelflow.TabIndex = 22;
            this.labelflow.Text = "0";
            // 
            // textBoxdebug
            // 
            this.textBoxdebug.Location = new System.Drawing.Point(12, 319);
            this.textBoxdebug.Multiline = true;
            this.textBoxdebug.Name = "textBoxdebug";
            this.textBoxdebug.Size = new System.Drawing.Size(261, 98);
            this.textBoxdebug.TabIndex = 24;
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(36, 183);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(87, 12);
            this.label8.TabIndex = 26;
            this.label8.Text = "Now incoming";
            // 
            // labelincomingCount
            // 
            this.labelincomingCount.AutoSize = true;
            this.labelincomingCount.Location = new System.Drawing.Point(155, 183);
            this.labelincomingCount.Name = "labelincomingCount";
            this.labelincomingCount.Size = new System.Drawing.Size(11, 12);
            this.labelincomingCount.TabIndex = 25;
            this.labelincomingCount.Text = "0";
            // 
            // timeout
            // 
            this.timeout.Tick += new System.EventHandler(this.timeout_Tick);
            // 
            // textBoxPacketSize
            // 
            this.textBoxPacketSize.Location = new System.Drawing.Point(86, 101);
            this.textBoxPacketSize.Name = "textBoxPacketSize";
            this.textBoxPacketSize.Size = new System.Drawing.Size(187, 21);
            this.textBoxPacketSize.TabIndex = 27;
            this.textBoxPacketSize.Text = "512";
            this.textBoxPacketSize.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(10, 104);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(70, 12);
            this.label9.TabIndex = 28;
            this.label9.Text = "packet size";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1475, 787);
            this.ControlBox = false;
            this.Controls.Add(this.label9);
            this.Controls.Add(this.textBoxPacketSize);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.labelincomingCount);
            this.Controls.Add(this.textBoxdebug);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.labelflow);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.labelNowIndex);
            this.Controls.Add(this.buttonstop);
            this.Controls.Add(this.buttonTest);
            this.Controls.Add(this.buttonExit);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.textBoxTerminal);
            this.Controls.Add(this.comboBoxBaudrate);
            this.Controls.Add(this.comboBoxPort);
            this.Controls.Add(this.buttonSerialPortSearch);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.buttonUpdateFileFind);
            this.Controls.Add(this.textBoxUpdateFileAddress);
            this.Controls.Add(this.buttonUpdateStart);
            this.Controls.Add(this.textBoxLog);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.buttonSerialOpen);
            this.Name = "Form1";
            this.Text = "FW Update";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button buttonSerialOpen;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox textBoxLog;
        private System.Windows.Forms.Button buttonUpdateStart;
        private System.Windows.Forms.TextBox textBoxUpdateFileAddress;
        private System.Windows.Forms.Button buttonUpdateFileFind;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Button buttonSerialPortSearch;
        private System.Windows.Forms.ComboBox comboBoxPort;
        private System.Windows.Forms.ComboBox comboBoxBaudrate;
        private System.Windows.Forms.TextBox textBoxTerminal;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Button buttonExit;
        private System.Windows.Forms.Button buttonTest;
        private System.Windows.Forms.Button buttonstop;
        private System.Windows.Forms.Label labelNowIndex;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label labelflow;
        private System.Windows.Forms.TextBox textBoxdebug;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label labelincomingCount;
        private System.Windows.Forms.Timer timeout;
        private System.Windows.Forms.TextBox textBoxPacketSize;
        private System.Windows.Forms.Label label9;
    }
}

