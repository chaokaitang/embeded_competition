Page({
  data: {
    productData: [
      { id: 0, name: '乒乓球', views: 150, purchases: 30, image: '/img/pingpong.jpeg' },
      { id: 1, name: '胶带', views: 120, purchases: 45, image: '/img/latex.png' },
      { id: 2, name: 'U盘', views: 200, purchases: 60, image: '/img/Ustick.jpeg' },
      { id: 3, name: 'STM32', views: 170, purchases: 50, image: '/img/STM32.jpg' }
    ],
    showDialog: false,
    modifyIndex: -1,
    newProductName: '',
    newImageTempPath: ''
  },

  onLoad: function () {
    // 页面加载时，从本地缓存中获取数据，确保显示最新的商品信息
    const productData = wx.getStorageSync('productData');
    if (productData) {
      this.setData({
        productData: productData
      });
    }
  },

  showModifyDialog: function (e) {
    const index = e.currentTarget.dataset.index;
    this.setData({
      showDialog: true,
      modifyIndex: index,
      newProductName: this.data.productData[index].name,
      newImageTempPath: this.data.productData[index].image
    });
  },

  inputProductName: function (e) {
    this.setData({
      newProductName: e.detail.value
    });
  },

  chooseImage: function () {
    const that = this;
    wx.chooseImage({
      count: 1,
      sizeType: ['original', 'compressed'],
      sourceType: ['album', 'camera'],
      success: function (res) {
        that.setData({
          newImageTempPath: res.tempFilePaths[0]
        });
      }
    });
  },

  cancelModify: function () {
    this.setData({
      showDialog: false,
      modifyIndex: -1,
      newProductName: '',
      newImageTempPath: ''
    });
  },

  confirmModify: function () {
    const index = this.data.modifyIndex;
    if (index !== -1) {
      const { newProductName, newImageTempPath } = this.data;
      const productData = this.data.productData;
      productData[index].name = newProductName;
      if (newImageTempPath) {
        productData[index].image = newImageTempPath;
      }
      this.setData({
        productData: productData,
        showDialog: false,
        modifyIndex: -1,
        newProductName: '',
        newImageTempPath: ''
      });

      // 更新本地缓存
      wx.setStorageSync('productData', productData);
    }
  },

  gotolinePage: function () {
    wx.navigateTo({
      url: '/pages/line/index?data=' + encodeURIComponent(JSON.stringify(this.data.productData))
    });
  }
});
