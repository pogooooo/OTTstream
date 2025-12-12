import {useEffect} from 'react';
import {useParams} from 'react-router-dom';
import styled from 'styled-components';
import BASE_URL from '../utils/config';
import {useSelector} from 'react-redux';
import type {RootState} from '../Store';
import {getProgress} from '../utils/api';

const VideoDetail = () => {
    const { id } = useParams<{ id: string }>();
    const videoId = id ? parseInt(id) : null;
    const isLoggedIn = useSelector((state: RootState) => state.user.isLoggedIn);
    const token = useSelector((state: RootState) => state.user.token);

    useEffect(() => {
        if (!videoId) {
            console.error("Invalid Video ID.");
            return;
        }

        // 3. 이어보기 위치 로드 및 iframe에 주입
        const loadVideo = async () => {
            const startPosition = await getProgress(videoId, token);

            // C 서버의 스트리밍 주소로 바로 연결
            // #t=[초] 형식으로 이어보기 위치를 URL에 추가합니다.
            const videoSourceUrl = `${BASE_URL}/video/play?id=${videoId}#t=${startPosition}`;

            const iframe = document.getElementById('videoPlayer') as HTMLIFrameElement;
            if (iframe) {
                iframe.src = videoSourceUrl;
            }
        };
        if (isLoggedIn) {
            loadVideo(); // 로그인된 경우에만 이어보기 로드
        } else {
            // 비로그인 시 0초부터 시작
            const iframe = document.getElementById('videoPlayer') as HTMLIFrameElement;
            if (iframe) {
                iframe.src = `${BASE_URL}/video/play?id=${videoId}#t=0`;
            }
        }
    }, [videoId, isLoggedIn, token]);

    if (!videoId) {
        return <DetailWrapper>유효하지 않은 영상입니다.</DetailWrapper>;
    }

    return (
        <DetailWrapper>
            <h1>영상 상세 페이지 (ID: {videoId})</h1>
            <p>로그인 상태: {isLoggedIn ? '로그인됨' : '비로그인'}</p>

            {/* C 서버가 반환하는 HTML 비디오 플레이어를 iframe으로 직접 삽입 */}
            <VideoFrame
                id="videoPlayer"
                title={`Video Player for ${videoId}`}
                frameBorder="0"
                allowFullScreen
            />
        </DetailWrapper>
    );
};

const DetailWrapper = styled.div`
    padding: 20px;
`;

const VideoFrame = styled.iframe`
    width: 800px; /* 실제 영상 플레이어 크기에 맞춰 조정 */
    height: 450px;
    border: 1px solid #333;
    display: block;
    margin-bottom: 20px;
`;

const InfoSection = styled.div`
    padding: 15px;
    border: 1px solid #ddd;
    border-radius: 8px;
`;

export default VideoDetail;
