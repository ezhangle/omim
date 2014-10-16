
#import <UIKit/UIKit.h>
#import "ProgressView.h"
#import "BadgeView.h"

#include "../../storage/storage_defines.hpp"

using namespace storage;

@class MapCell;
@protocol MapCellDelegate <NSObject>

@optional
- (void)mapCellDidStartDownloading:(MapCell *)cell;
- (void)mapCellDidCancelDownloading:(MapCell *)cell;

@end

@interface MapCell : UITableViewCell

@property (nonatomic, readonly) UILabel * titleLabel;
@property (nonatomic, readonly) UILabel * subtitleLabel;
@property (nonatomic, readonly) UILabel * sizeLabel;
@property (nonatomic, readonly) BadgeView * badgeView;

@property (nonatomic) BOOL parentMode;

@property (nonatomic) TStatus status;
@property (nonatomic) TMapOptions options;
@property (nonatomic) double downloadProgress;

@property (nonatomic, weak) id <MapCellDelegate> delegate;

- (void)setStatus:(TStatus)status options:(TMapOptions)options animated:(BOOL)animated;
- (void)setDownloadProgress:(double)downloadProgress animated:(BOOL)animated;

+ (CGFloat)cellHeight;

@end